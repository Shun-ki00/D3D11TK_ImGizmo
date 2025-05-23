#include "pch.h"
#include "Game/Scene.h"
#include "Framework/CommonResources.h"
#include "Framework/DebugCamera.h"
#include "imgui/ImGuizmo.h"


/// <summary>
/// コンストラクタ
/// </summary>
Scene::Scene()
{
	// インスタンスを取得する
	m_commonResources = CommonResources::GetInstance();
	m_device          = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDevice();
	m_context         = CommonResources::GetInstance()->GetDeviceResources()->GetD3DDeviceContext();
}

/// <summary>
/// 初期化処理
/// </summary>
void Scene::Initialize()
{

	// カメラの作成
	m_camera = std::make_unique<DebugCamera>();
	m_camera->Initialize(1280, 720);

	// 初期化
	m_position   = DirectX::SimpleMath::Vector3::Zero;
	m_rotation = DirectX::SimpleMath::Quaternion::Identity;
	m_scale      = DirectX::SimpleMath::Vector3::One;
	m_world =
		DirectX::SimpleMath::Matrix::CreateScale(m_scale) *
		DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_rotation) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	m_gridMatrix = DirectX::SimpleMath::Matrix::Identity;
	this->MatrixToFloatArrayColumnMajor(m_gridMatrix, m_arrayGridMatrix);

	// ギズモ操作の種類初期化
	m_operation = ImGuizmo::TRANSLATE;
	m_mode      = ImGuizmo::LOCAL;
}

/// <summary>
/// 更新処理
/// </summary>
/// <param name="elapsedTime">経過時間</param>
void Scene::Update(const float& elapsedTime)
{
	UNREFERENCED_PARAMETER(elapsedTime);

	// カメラの更新処理
	auto io = ImGui::GetIO();
	if (!io.WantCaptureMouse)
	{
		m_camera->Update();
		m_commonResources->SetViewMatrix(m_camera->GetViewMatrix());
	}

}

/// <summary>
/// 描画処理
/// </summary>
void Scene::Render()
{

	// imgui用に配列行列
	float viewArrayMatrix[16], projectionArrayMatrix[16], worldArrayMatrix[16];

	DirectX::SimpleMath::Matrix viewMatrix       = m_camera->GetViewMatrix();
	DirectX::SimpleMath::Matrix projectionMatrix = m_commonResources->GetProjectionMatrix();

	// 変換
	this->MatrixToFloatArrayColumnMajor(viewMatrix, viewArrayMatrix);
	this->MatrixToFloatArrayColumnMajor(projectionMatrix, projectionArrayMatrix);
	this->MatrixToFloatArrayColumnMajor(m_world, worldArrayMatrix);

	// ギズモの準備
	ImGuizmo::BeginFrame(); 

	// 画面サイズを設定
	ImGuiViewport* mainViewport = ImGui::GetMainViewport();
	ImGuizmo::SetRect(
		mainViewport->Pos.x,
		mainViewport->Pos.y,
		mainViewport->Size.x,
		mainViewport->Size.y
	);

	// グリッドを描画
	ImGuizmo::DrawGrid(viewArrayMatrix, projectionArrayMatrix, m_arrayGridMatrix, 20);

	// ビューキューブ
	ImVec2 viewCubePos = ImVec2(mainViewport->Pos.x + mainViewport->Size.x - 128.0f, mainViewport->Pos.y);
	ImVec2 viewCubeSize = ImVec2(128, 128);
	ImGuizmo::ViewManipulate(viewArrayMatrix, 10.0f, viewCubePos, viewCubeSize, IM_COL32(0, 0, 0, 0));

	// キューブを描画
	ImGuizmo::DrawCubes(viewArrayMatrix, projectionArrayMatrix, worldArrayMatrix, 1);
	// ギズモを描画
	ImGuizmo::Manipulate(viewArrayMatrix, projectionArrayMatrix,m_operation,m_mode, worldArrayMatrix);

	DirectX::SimpleMath::Matrix view;
	this->FloatArrayToMatrixColumnMajor(&view, viewArrayMatrix);
	m_camera->SetViewMatrix(view);

	// 変換
	this->FloatArrayToMatrixColumnMajor(&m_world, worldArrayMatrix);

	// ギズモ操作された worldArrayMatrix → 各変数に反映
	float t[3], r[3], s[3]; // translation, rotation (degrees), scale
	ImGuizmo::DecomposeMatrixToComponents(worldArrayMatrix, t, r, s);

	auto io = ImGui::GetIO();
	if (io.WantCaptureMouse)
	{
		m_position = DirectX::SimpleMath::Vector3(t[0], t[1], t[2]);
		m_scale = DirectX::SimpleMath::Vector3(s[0], s[1], s[2]);
		m_rotation = DirectX::SimpleMath::Quaternion::CreateFromRotationMatrix(m_world);
	}

	// エディタ
	EditTransform();
}

/// <summary>
/// 終了処理
/// </summary>
void Scene::Finalize() {}


// SimpleMathの Matrix を float[16]配列 に変換する
void Scene::MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat)
{
	memcpy(mat, &matrix, sizeof(float) * 16);
}

// SimpleMathの float[16]配列 を Matrix に変換する
void Scene::FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix, const float* mat)
{
	memcpy(matrix, mat, sizeof(DirectX::SimpleMath::Matrix));
}

/// <summary>
/// Transformのエディタを描画
/// </summary>
void Scene::EditTransform()
{
	using namespace DirectX::SimpleMath;

	// キーボード切替
	if (ImGui::IsKeyPressed(ImGuiKey_T)) m_operation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(ImGuiKey_R)) m_operation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(ImGuiKey_S)) m_operation = ImGuizmo::SCALE;

	// 操作タイプUI
	if (ImGui::RadioButton(u8"位置", m_operation == ImGuizmo::TRANSLATE))
		m_operation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton(u8"回転", m_operation == ImGuizmo::ROTATE))
		m_operation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton(u8"スケール", m_operation == ImGuizmo::SCALE))
		m_operation = ImGuizmo::SCALE;

	if (m_operation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton(u8"ローカル", m_mode == ImGuizmo::LOCAL))
			m_mode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton(u8"ワールド", m_mode == ImGuizmo::WORLD))
			m_mode = ImGuizmo::WORLD;
	}

	// Manipulate された transformMat を分解して各要素に反映
	float t[3] = {m_position.x , m_position.y , m_position.z },
		  s[3] = {m_scale.x ,m_scale.y ,m_scale.z };

	// オイラー角を分解して表示用に使う（変換はしない）
	DirectX::SimpleMath::Vector3 euler = m_rotation.ToEuler(); // radians
	float r[3] = {
		DirectX::XMConvertToDegrees(euler.x),
		DirectX::XMConvertToDegrees(euler.y),
		DirectX::XMConvertToDegrees(euler.z)
	};

	// GUI 入力で数値変更可能
	
	// 表示＋編集（GUIから触られたらそのときだけ反映）
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

	// 各要素更新
	m_position = Vector3(t[0], t[1], t[2]);
	m_scale = Vector3(s[0], s[1], s[2]);

	// ワールド行列更新
	m_world = DirectX::SimpleMath::Matrix::CreateScale(m_scale) *
		DirectX::SimpleMath::Matrix::CreateFromQuaternion(m_rotation) *
		DirectX::SimpleMath::Matrix::CreateTranslation(m_position);
	
}