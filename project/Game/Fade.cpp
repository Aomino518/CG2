#include "Fade.h"

void Fade::Init() {
	// テクスチャ
	tex_ = TextureManager::GetInstance()->Load("resources/texWhite.png");

	// スプライト
	sprite_ = std::make_unique<Sprite>();
	sprite_->Init();
	auto app = Application::GetInstance();
	sprite_->Create(tex_, { 0.0f, 0.0f }, Color::WHITE, { float(app->GetWidth()), float(app->GetHeight()) });
}

void Fade::Update() {
	// フェード状態による分岐
	switch (status_) {
	case Status::None:
		// なにもしない
		break;
	case Status::FadeIn:
		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 0.0fから1.0fの間で、経過時間がフェード継続時間に近づくほどアルファ値を小さくする
		sprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, std::clamp(1.0f - counter_ / duration_, 0.0f, 1.0f)));

		break;
	case Status::FadeOut:
		// 1フレーム分の秒数をカウントアップ
		counter_ += 1.0f / 60.0f;
		// フェード継続時間に達したら打ち止め
		if (counter_ >= duration_) {
			counter_ = duration_;
		}
		// 0.0fから1.0fの間で、経過時間がフェード継続時間に近づくほどアルファ値を大きくする
		sprite_->SetColor(Vector4(0.0f, 0.0f, 0.0f, std::clamp(counter_ / duration_, 0.0f, 1.0f)));

		break;
	}

	sprite_->Update();
}

void Fade::Draw() {
	if (status_ == Status::None) {
		return;
	}

	SpriteCommon::GetInstance()->DrawCommon();
	sprite_->Draw();
}

void Fade::Start(Status status, float duration) {
	status_ = status;
	duration_ = duration;
	counter_ = 0.0f;
}

void Fade::Stop() {
	status_ = Status::None;
}

bool Fade::IsFinished() const {
	switch (status_) {
	case Status::FadeIn:
	case Status::FadeOut:
		if (counter_ >= duration_) {
			return true;
		} else {
			return false;
		}
	}
	return true;
}
