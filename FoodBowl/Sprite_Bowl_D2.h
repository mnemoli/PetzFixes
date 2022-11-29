#pragma once
#include "Sprite_Bowl.h"
#include "Area.h"
class Sprite_Bowl_D2 : public Sprite_Bowl
{
public:
	void InitToyExtraCode(bool, Host*) override;
	void DoPhysics() override;
	void PlayFoleySound(EFoleySound, EFlavor) override;
};

