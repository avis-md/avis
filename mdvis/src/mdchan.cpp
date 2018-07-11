#include "mdchan.h"
#include "res/resdata.h"

Texture* MdChan::texs[] = {};
float MdChan::blink = 0, MdChan::t4 = 0, MdChan::t4d = 0;

void MdChan::Init() {
	texs[0] = new Texture(res::mdc_hd1_png, res::mdc_hd1_png_sz);
	texs[1] = new Texture(res::mdc_hd2_png, res::mdc_hd2_png_sz);
	texs[2] = new Texture(res::mdc_leg_png, res::mdc_leg_png_sz, TEX_FILTER_BILINEAR, TEX_WRAP_CLAMP);
	texs[3] = new Texture(res::mdc_t4p_png, res::mdc_t4p_png_sz, TEX_FILTER_BILINEAR, TEX_WRAP_CLAMP);
	blink = Random::Range(0.5f, 2.5f);
	t4 = Random::Range(0, 30.0f);
}

void MdChan::Draw(Vec2 pos) {
	pos.y += cosf(powf(fmodf(Time::time * 0.6f, 1.0f), 0.6f) * 2 * PI) * Display::height * 0.03f;
	float sz = Display::height * 0.05f;
	blink -= Time::delta;
	if (blink < -0.1f) {
		blink = Random::Range(0.5f, 2.5f);
		t4 = Random::Range(0, 30.0f);
	}
	t4d = Lerp(t4d, t4, Time::delta * 5);
	float ang = 45 + 15 * cosf(powf(fmodf(Time::time * 0.6f + 0.2f, 1.0f), 0.7f) * 2 * PI);
	Engine::RotateUI(ang, pos);
	UI::Texture(pos.x - sz, pos.y, sz * 2, sz * 2, texs[2]);
	Engine::RotateUI(-2*ang, pos);
	UI::Texture(pos.x - sz, pos.y, sz * 2, sz * 2, texs[2]);
	Engine::ResetUIMatrix();
	Engine::RotateUI(t4d, pos);
	UI::Texture(pos.x - sz, pos.y - sz, sz * 2, sz * 2, texs[3]);
	Engine::ResetUIMatrix();
	UI::Texture(pos.x - sz, pos.y - sz, sz * 2, sz * 2, (blink < 0) ? texs[1] : texs[0]);
}