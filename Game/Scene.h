#pragma once
#include <future>
#include "Framework/DebugCamera.h"

class CommonResources;
class DebugCamera;

class Scene
{
public:

	// コンストラクタ
	Scene();
	// デストラクタ
	~Scene() = default;

public:

	// 初期化処理
	void Initialize();
	// 更新処理
	void Update(const float& elapsedTime);
	// 描画処理
	void Render();
	// 終了処理
	void Finalize();

private:

	// TK提供のラッパーから配列に変換
	void MatrixToFloatArrayColumnMajor(const DirectX::SimpleMath::Matrix& matrix, float* mat);
	// TK提供のラッパーから配列に変換
	void FloatArrayToMatrixColumnMajor(DirectX::SimpleMath::Matrix* matrix,const float* mat);

	// Transformエディタ
	void EditTransform();

private:

	// 共有リソース
	CommonResources* m_commonResources;

	// デバッグカメラ
	std::unique_ptr<DebugCamera> m_camera;

	// デバイス
	ID3D11Device1* m_device;
	// コンテキスト
	ID3D11DeviceContext1* m_context;
	
	// モデル座標
	DirectX::SimpleMath::Vector3 m_position;
	// モデル回転
	DirectX::SimpleMath::Quaternion m_rotation;
	// モデルスケール
	DirectX::SimpleMath::Vector3 m_scale;

	// モデルワールド行列
	DirectX::SimpleMath::Matrix m_world;

	// 原点行列
	DirectX::SimpleMath::Matrix m_gridMatrix;
	float m_arrayGridMatrix[16];

	// ギズモのモード
	ImGuizmo::OPERATION m_operation;
	ImGuizmo::MODE m_mode;
};