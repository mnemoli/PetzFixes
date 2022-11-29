#include "pch.h"
#include "Sprite_Post_B1.h"
#include "Ledge.h"
#include "Area.h"
#include "Host.h"
#include <random>

using namespace Petz;

namespace Post {
	int randrange(int min, int max) {
		return ((double)rand() / RAND_MAX) * (max - min) + min;
	}
}

void Sprite_Post_B1::InitToyExtraCode(bool, Host*)
{
	SetAdjValue(Toyz, 23);
	SetAdjValue(Color, Brown);
	SetAdjValue(Flavor, Wood);
	SetAdjValue(Size, 100);
	SetAdjValue(Mass, Post::randrange(90, 100));
	SetAdjValue(Friction, Post::randrange(28, 32));
	SetAdjValue(EAdj::Soft, Post::randrange(15, 25));
	SetAdjValue(Fuzzy, Post::randrange(55, 65));
	SetAdjValue(Bounce, Post::randrange(10, 20));

}

void Sprite_Post_B1::AddInHostExtraCode(Host& host)
{
	Sprite_Post::AddInHostExtraCode(host);
	auto area = dynamic_cast<Area*>(&host);
	if (area != nullptr) {
		XTRect<> rect{ 0, 0, 0, 0 };
		scratchLedge = new Ledge(rect, false, false);
		SetLedge(scratchLedge);
	}
}

void Sprite_Post_B1::RemInHostExtraCode(Host& host, const char* str, const char* str2)
{
	Sprite_Post::RemInHostExtraCode(host, str, str2);
}

void Sprite_Post_B1::RunUpdate()
{
	Sprite_Post::RunUpdate();
	if (scratchLedge != nullptr) {
		const auto& bounds = GetDrawRect();
		auto ledgeBounds = ComputeLedgeRect(bounds);
		scratchLedge->SetBounds(ledgeBounds);
	}
}

XTRect<> Sprite_Post_B1::ComputeLedgeRect(XTRect<> rec)
{
	rec.y1 -= 7;
	rec.y2 = rec.y1 + 5;
	rec.x1 += 20;
	rec.x2 -= 20;
	return rec;
}
