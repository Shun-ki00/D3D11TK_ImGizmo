#include "pch.h"
#include "Game/Scene.h"
#include "Framework/CommonResources.h"
#include "Framework/DebugCamera.h"
#include "imgui/ImGuizmo.h"


/// <summary>
/// �R���X�g���N�^
/// </summary>
Scene::Scene()
{
	// �C���X�^���X���擾����
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
}

/// <summary>
/// ����������
/// </summary>
void Scene::Initialize()
{

	// �J�����̍쐬
	m_camera = std::make_unique<DebugCamera>();
	m_camera->Initialize(1280, 720);

	// ������
	m_position   = DirectX::SimpleMath::Vector3::Zero;
	m_rotation = DirectX::SimpleMath::Quaternion::Identity;
	m_scale      = DirectX::SimpleMath::Vector3::One;
	m_world =
		DirectX::SimpleMath::Matrix::CreateScale(m_scale) *
		DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_rotation) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	m_gridMatrix = DirectX::SimpleMath::Matrix::Identity;
	this->MatrixToFloatArrayColumnMajor(m_gridMatrix, m_arrayGridMatrix);

	// �M�Y������̎�ޏ�����
	m_operation = ImGuizmo::TRANSLATE;
	m_mode      = ImGuizmo::LOCAL;
}

/// <summary>
/// �X�V����
/// </summary>
/// <param name="elapsedTime">�o�ߎ���</param>
void Scene::Update(const float& elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	// �J�����̍X�V����
	auto io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		m_camera->Update();
		m_commonResources->SetViewMatrix(m_camera->GetViewMatrix());
	}

}

/// <summary>
/// �`�揈��
/// </summary>
void Scene::Render()
{

	// imgui�p�ɔz��s��
	float viewArrayMatrix[16], projectionArrayMatrix[16], worldArrayMatrix[16];

	DirectX::SimpleMath::Matrix viewMatrix       = m_camera->GetViewMatrix();
	DirectX::SimpleMath::Matrix projectionMatrix = m_commonResources->GetProjectionMatrix();

	// �ϊ�
	this->MatrixToFloatArrayColumnMajor(viewMatrix, viewArrayMatrix);
	this->MatrixToFloatArrayColumnMajor(projectionMatrix, projectionArrayMatrix);
	this->MatrixToFloatArrayColumnMajor(m_world, worldArrayMatrix);

	// �M�Y���̏���
	ImGuizmo::BeginFrame(); 

	// ��ʃT�C�Y��ݒ�
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGuizmo::SetRect(
		mainViewport->Pos.x,
		mainViewport->Pos.y,
		mainViewport->Size.x,
		mainViewport->Size.y
	);

	// �O���b�h��`��
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 20);

	// �r���[�L���[�u
	ImVec2 viewCubePos = ImVec2(mainViewport->Pos.x + mainViewport->Size.x - 128.0f, mainViewport->Pos.y);
	ImVec2 viewCubeSize = ImVec2(128, 128);
	ImGuizmo::ViewManipulate(viewArrayMatrix, 10.0f, viewCubePos, viewCubeSize, IM_COL32(0, 0, 0, 0));

	// �L���[�u��`��
	ImGuizmo::DrawCubes(viewArrayMatrix, projectionArrayMatrix, worldArrayMatrix, 1);
	// �M�Y����`��
	ImGuizmo::Manipulate(viewArrayMatrix, projectionArrayMatrix,m_operation,m_mode, worldArrayMatrix);

	DirectX::SimpleMath::Matrix view;
	this->FloatArrayToMatrixColumnMajor(&view, viewArrayMatrix);
	m_camera->SetViewMatrix(view);

	// �ϊ�
	this->FloatArrayToMatrixColumnMajor(&m_world, worldArrayMatrix);

	// �M�Y�����삳�ꂽ worldArrayMatrix �� �e�ϐ��ɔ��f
	float t[3], r[3], s[3]; // translation, rotation (degrees), scale
	ImGuizmo::DecomposeMatrixToComponents(worldArrayMatrix, t, r, s);

	auto io = ImGui::GetIO();
	if (io.WantCaptureMouse)
	{
		m_position = DirectX::SimpleMath::Vector3(t[0], t[1], t[2]);
		m_scale = DirectX::SimpleMath::Vector3(s[0], s[1], s[2]);
		m_rotation = DirectX::SimpleMath::Quaternion::CreateFromRotationMatrix(m_world);
	}

	// �G�f�B�^
	EditTransform();
}

/// <summary>
/// �I������
/// </summary>
void Scene::Finalize() {}


// SimpleMath�� Matrix �� float[16]�z�� �ɕϊ�����
void Scene::MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat)
{
	memcpy(mat, &matrix, sizeof(float) * 16);
}

// SimpleMath�� float[16]�z�� �� Matrix �ɕϊ�����
void Scene::FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix, const float* mat)
{
	memcpy(matrix, mat, sizeof(DirectX::SimpleMath::Matrix));
}

/// <summary>
/// Transform�̃G�f�B�^��`��
/// </summary>
void Scene::EditTransform()
{
	using namespace DirectX::SimpleMath;

	// �L�[�{�[�h�ؑ�
	if (ImGui::IsKeyPressed(ImGuiKey_T)) m_operation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) m_operation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_S)) m_operation = ImGuizmo::SCALE;

	// ����^�C�vUI
	if (ImGui::RadioButton(u8"�ʒu", m_operation == ImGuizmo::TRANSLATE))
		m_operation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton(u8"��]", m_operation == ImGuizmo::ROTATE))
		m_operation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton(u8"�X�P�[��", m_operation == ImGuizmo::SCALE))
		m_operation = ImGuizmo::SCALE;

	if (m_operation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton(u8"���[�J��", m_mode == ImGuizmo::LOCAL))
			m_mode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton(u8"���[���h", m_mode == ImGuizmo::WORLD))
			m_mode = ImGuizmo::WORLD;
	}

	// Manipulate ���ꂽ transformMat �𕪉����Ċe�v�f�ɔ��f
	float t[3] = {m_position.x , m_position.y , m_position.z },
		  s[3] = {m_scale.x ,m_scale.y ,m_scale.z };

	// �I�C���[�p�𕪉����ĕ\���p�Ɏg���i�ϊ��͂��Ȃ��j
	DirectX::SimpleMath::Vector3 euler = m_rotation.ToEuler(); // radians
	float r[3] = {
		DirectX::XMConvertToDegrees(euler.x),
		DirectX::XMConvertToDegrees(euler.y),
		DirectX::XMConvertToDegrees(euler.z)
	};

	// GUI ���͂Ő��l�ύX�\
	
	// �\���{�ҏW�iGUI����G��ꂽ�炻�̂Ƃ��������f�j
	bool changed = false;
	ImGui::InputFloat3("Position", t);
	changed |= ImGui::InputFloat3("Rotation", r);
	ImGui::InputFloat3("Scale", s);

	if (changed)
	{
		m_rotation = Quaternion::CreateFromYawPitchRoll(
			DirectX::XMConvertToRadians(r[1]), // yaw
			DirectX::XMConvertToRadians(r[0]), // pitch
			DirectX::XMConvertToRadians(r[2])  // roll
		);
	}

	// �e�v�f�X�V
	m_position = Vector3(t[0], t[1], t[2]);
	m_scale = Vector3(s[0], s[1], s[2]);

	// ���[���h�s��X�V
	m_world = DirectX::SimpleMath::Matrix::CreateScale(m_scale) *
		DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_rotation) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	
}