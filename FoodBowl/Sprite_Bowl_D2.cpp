#include "pch.h"
#include "Sprite_Bowl_D2.h"
#include "Oberon.h"
#include <random>
#include <typeinfo>

namespace BowlD2 {
	int randrange(int min, int max) {
		return ((double)rand() / RAND_MAX) * (max - min) + min;
	}
}
using namespace Petz;
using namespace BowlD2;

void Sprite_Bowl_D2::InitToyExtraCode(bool a, Host* host)
{
	if (a) {
		SetAdjValue(Toyz, 5);
		SetAdjValue(Color, Blue);
		SetAdjValue(Flavor, Beef);
		SetAdjValue(Size, randrange(75, 85));
		SetAdjValue(Mass, randrange(90, 100));
		SetAdjValue(Friction, randrange(30, 34));
		SetAdjValue(Tasty, randrange(90, 100));
		SetAdjValue(Edible, randrange(75, 85));
		SetAdjValue(Fatty, randrange(5, 15));
		SetAdjValue(Bounce, randrange(15, 25));
		SetAdjValue(BadNoisy, randrange(15, 25));
	}
	foodAmount = 5;
}

void Sprite_Bowl_D2::DoPhysics()
{
	if (g_Oberon.area != nullptr) {
		auto p = GetDrawPoint();
		auto h = g_Oberon.area->GetHorizonAt(p);
		if (p.y < h) {
			Sprite_Bowl::DoPhysics();
		}
		else {
			Sprite_Bowl::Do2DPhysics();
		}
	}
	else {
		Sprite_Bowl::DoPhysics();
	}

}

void Sprite_Bowl_D2::PlayFoleySound(EFoleySound sound, EFlavor flavor)
{
	if (sound != 1 && sound != 4) {
		AlpoSprite::PlayFoleySound(sound, flavor);
	}
	else {
		PlaySurfaceContactSound();
		auto p = GetDrawPoint();
		auto s = g_Oberon.area->GetSurfaceAdjAt(p);
		double speed = abs(GetSpeed());
		if (s != 3 && speed <= 10.0f) {
			return;
		}
		PlaySoundIndex(0,0,100,0,0);
	}
}
