// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include "Utils.h"
#include "XDownload.h"
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <memoryapi.h>
#include "XBallz.h"
#include "Linez.h"
#include <math.h>
#include <algorithm>
#include "XSprite.h"
#include <string>

int __declspec(dllimport) NormalizeAngleFunc(int);

void* operator new (size_t size) { return PetzNew(size); };
void* operator new[](size_t size) { return PetzNew(size); };
void operator delete(void* ptr) { PetzDelete(ptr); };
void operator delete[](void* ptr) { PetzDelete(ptr); };

void hotpatch(void* target, void* replacement)
{
	void* page = (void*)((uintptr_t)target & ~0xfff);
	DWORD oldprotect;
	VirtualProtect(page, 4096, PAGE_READWRITE, &oldprotect);
	uint32_t rel = (char*)replacement - (char*)target - 5;
	union {
		uint8_t bytes[8];
		uint64_t value;
	} instruction = { {0xe9, rel >> 0, rel >> 8, rel >> 16, rel >> 24} };
	*(uint64_t*)target = instruction.value;
	VirtualProtect(page, 4096, oldprotect, &oldprotect);
}

void ApplyPatch() {
	auto lib = LoadLibraryA("Petz 4.exe");
	auto fun = GetProcAddress(lib, "?DisplayBallzFrame@XBallz@@QAEXPAVXDrawPort@@PBU?$XTRect@HJ@@PAVBallState@@@Z");
	// faff about to turn function pointer into void*
	auto fun2 = &XBallz::DisplayBallzFrame;
	auto fptr = &fun2;
	auto ff = (void*&)*fptr;
	hotpatch((void*)fun, ff);
	FreeLibrary(lib);
}

BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    DisableThreadLibraryCalls(hModule);
    switch (ul_reason_for_call)
    {
	case DLL_PROCESS_ATTACH: 
		ApplyPatch();
		break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

bool GetLoadInfo(pfvector<LoadInfo>& loadInfoVector, ELoadType loadType, char const* someString, XLibraryList* xLibraryList) {
    return false;
}

long GetDLLVersion() {
    return 0x44000007;
}

XTVector3<long, long> averagevec(XTVector3<long, long>& one, XTVector3<long, long>& two) {
	return XTVector3<long, long> {
		(one.x + two.x) / 2,
		(one.y + two.y) / 2,
		(one.z + two.z) / 2
	};
}

namespace patch {
	const float TWOPI = 2 * 3.14159265358979323846;
	const double SINCOSVAL = 0.02454369260617026;
	void DrawEyeball(XBallz* xballz, XDrawPort* drawport, BallFrameEx const* ballframeex, BallState const* ballstate, int ballno, CircleRenderBlock const& circlerenderblock, int ballsize, XTPoint<int> const& center)
	{
		auto lnz = xballz->linez;
		int irisno;
		int eyeheight;
		int eyelidtilt;
		if (ballno == lnz->eye2ballno) {
			irisno = lnz->iris2ballno;
			eyeheight = ballstate->leyeheight;
			eyelidtilt = ballstate->leyelidtilt;
		}
		else {
			irisno = lnz->iris1ballno;
			eyeheight = ballstate->reyeheight;
			eyelidtilt = -ballstate->reyelidtilt;
		}
		if (eyeheight < 0) {
			eyeheight = 0;
		}
		else if (eyeheight > 101) {
			eyeheight = 100;
		}
		int irissize = ballframeex->ballsize[irisno];
		int eyeballsize = ballframeex->ballsize[ballno];
		const auto& irisrotation = ballframeex->posrotinfo[irisno];
		const auto& eyeballrotation = ballframeex->posrotinfo[ballno];
		auto irisx = irisrotation.x - eyeballrotation.x;
		auto irisy = irisrotation.y - eyeballrotation.y;
		if (ballstate->eyetrackingmode == 1) {
			// track object
			irisx = irisx + ballstate->eyetargetx;
			irisy = irisy + ballstate->eyetargety;
			if (irisx != 0 && irisy != 0) {
				// clamp within eyeball
				auto irisvectorlength = sqrt(irisy * irisy + irisx * irisx);
				if (irisvectorlength >= eyeballsize) {
					irisx = round(irisx * (eyeballsize / irisvectorlength));
					irisy = round(irisy * (eyeballsize / irisvectorlength));
				}
			}
			
		}
		else if (ballstate->eyetrackingmode == 2) {
			//stare into player's soul
			irisx = 0;
			irisy = 0;
		}

		CircleRenderBlock drawblock = CircleRenderBlock(lnz->rendermode);
		drawblock.fuzz = lnz->fuzzoverride[irisno];
		drawblock.colorindex = lnz->colorindexarray[irisno];
		drawblock.outlinecolor = lnz->outlinecolorarray[irisno];
		drawblock.textureinfo1 = 0;
		drawblock.tex2 = 0;
		drawblock.tex3 = 0;
		drawblock.outlinetype = lnz->outlinearray[irisno];
		drawblock.notexturerotate = 0;
		drawblock.eyelidangle = 0;
		drawblock.eyelidheight = 0;
		drawblock.rect.y2 = center.y + irisy;
		drawblock.rect.x2 = center.x + irisx;
		drawblock.rect.x1 = drawblock.rect.x2 - irissize;
		drawblock.rect.y1 = drawblock.rect.y2 - irissize;
		drawblock.rect.y2 += irissize;
		drawblock.rect.x2 += irissize;
		drawblock.clipwithin = &circlerenderblock;
		// custom - allow textured iris
		xballz->SetBallTextureInfo(drawblock, *ballstate, ballframeex->posrotinfo, irisno);

		// if we have some eyelid showing
		if (eyeheight != 0) {
			auto rotvec = XTVector3<long, long>{
				0,
				256,
				0
			};
			auto rotvec2 = XTVector3<long, long>{
				0,
				256,
				0
			};
			xballz->RotateBallPoint(rotvec, *ballstate, lnz->eye1ballno, ballframeex->posrotinfo[lnz->eye1ballno]);
			xballz->RotateBallPoint(rotvec2, *ballstate, lnz->eye2ballno, ballframeex->posrotinfo[lnz->eye2ballno]);
			auto upavg = averagevec(rotvec, rotvec2);
			rotvec = XTVector3<long, long>{
				256,
				0,
				0
			};
			rotvec2 = XTVector3<long, long>{
				256,
				0,
				0
			};
			xballz->RotateBallPoint(rotvec, *ballstate, lnz->eye1ballno, ballframeex->posrotinfo[lnz->eye1ballno]);
			xballz->RotateBallPoint(rotvec2, *ballstate, lnz->eye2ballno, ballframeex->posrotinfo[lnz->eye2ballno]);
			auto leftavg = averagevec(rotvec, rotvec2);


			auto veclength = round(sqrt(upavg.y * upavg.y + upavg.x * upavg.x));
			int latitudeshifted = 0;
			if (veclength == 0 && upavg.z == 0) {
				// latitude is 0
			}
			else {
				//get latitude
				auto latitudeofup = atan2(upavg.z, veclength);
				latitudeshifted = round((latitudeofup * 256.0f) / TWOPI);
			}
			latitudeshifted = latitudeshifted * 4;
			auto abslatitude = abs(latitudeshifted);
			int finalanglex = 0;
			// eyes looking forward
			if (abslatitude < 65) {
				if (upavg.y == 0 && upavg.x == 0) {
					finalanglex = 0;
				}
				else {
					auto somefpatan = atan2(upavg.x, upavg.y); // or is this x?
					finalanglex = -round((somefpatan * 256.0f) / TWOPI); 
				}
			}
			// eyes looking up/down a bit
			else if (abslatitude < 192) {
				signed int tempvaly = 0;
				signed int tempvalx = 0;
				if (upavg.y != 0 || upavg.x != 0) {
					auto somefpatan = atan2(upavg.x, upavg.y);
					tempvaly = round((somefpatan * 256.0f) / TWOPI);
				}
				if (leftavg.x != 0 || leftavg.y != 0) {
					auto somefpatan = atan2(leftavg.y, leftavg.x);
					tempvalx = round((somefpatan * 256.0f) / TWOPI);
				}
				tempvalx = tempvalx * abslatitude - (256 - abslatitude) * tempvaly;
				if (tempvalx < 0) {
					tempvalx += 256;
				}
				finalanglex = tempvalx / 256;
			}
			// eyes looking up/down a lot
			else {
				if (leftavg.x == 0 && leftavg.y == 0) {
					finalanglex = 0;
				}
				else {
					auto somefpatan = atan2(leftavg.y, leftavg.x);
					finalanglex = round((somefpatan * 256.0f) / TWOPI);
				}
			}
			// eyelid tilt stuff
			auto eyeopen = 100 - eyeheight;
			auto eyeopenpct = (eyelidtilt * eyeopen) / 100;
			int finaleyeheight = 0;
			if (abs(eyeopenpct) < 10) {
				auto veclength = round(sqrt(leftavg.y * leftavg.y + leftavg.x * leftavg.x));
				double latitude2 = 0;
				int extrastuff = 0;
				if (veclength == 0 && leftavg.z == 0) {
					latitude2 = 0;
				}
				else {
					latitude2 = atan2(leftavg.z, veclength);
					latitude2 = round((latitude2 * 256.0f) / TWOPI);
					extrastuff = latitude2;
				}
				if (eyeopenpct < 1) {
					latitude2 = -latitude2;

				}
				eyeopenpct = ((latitude2 + 27) * eyeopenpct) / 27 - extrastuff / 3;
				if (0 < eyeopenpct) {
					// needs cleanup to clarify shifting
					int eyeheightintermediate = (eyeopenpct * 3) >> 31;
					eyeheightintermediate = ((eyeopenpct * 3 ^ (int)eyeheightintermediate) - eyeheightintermediate) * eyeopen * eyeopen;
					finaleyeheight = (eyeheightintermediate * 0.00000156 + 1.0) * (float)eyeheight;
					eyeheight = round(finaleyeheight);

				}
			}
			int eyeopenlat = (100 - eyeheight) * latitudeshifted;
			if (eyeopenlat < 0) {
				eyeopenlat += 256;
			}
			eyeheight = eyeheight + eyeopenlat / 256;
			// clamp eyelid height
			if (eyeheight < 1) {
				eyeheight = 1;
			}
			else if (eyeheight > 101) {
				eyeheight = 100;
			}

			auto normalizedangle = NormalizeAngleFunc(finalanglex + eyeopenpct);
			auto sinval = sinf(normalizedangle * SINCOSVAL);
			auto cosval = cosf(normalizedangle * SINCOSVAL);
			auto somepct = ((circlerenderblock.rect.y2 - circlerenderblock.rect.y1) * eyeheight) / 100;
			auto someval = -(irisx * sinval + irisy * -cosval + (somepct - ballsize));

			// start pushing the iris around
			int irispushdownx = 0;
			int irispushdowny = 0;
			if (someval < 0.0) {
				auto cart1 = someval * sinval;
				auto cart2 = someval * -cosval;
				irispushdownx = round(cart1);
				irispushdowny = round(cart2);
				drawblock.rect.x1 = center.x - irissize + irispushdownx + irisx;
				drawblock.rect.x2 = center.x + irissize + irispushdownx + irisx;
				drawblock.rect.y1 = center.y - irissize + irispushdowny + irisy;
				drawblock.rect.y2 = center.y + irissize + irispushdowny + irisy;
			}
			// draw iris
			drawport->XFillCircleEx(&drawblock);

			// draw eyelid
			drawblock.rect = circlerenderblock.rect;
			drawblock.colorindex = lnz->eyelidcolor1;
			drawblock.fuzz = 0;
			drawblock.clipwithin = 0;
			drawblock.eyelidheight = somepct;
			drawblock.outlinecolor = 244;
			drawblock.outlinetype = -4;
			drawblock.eyelidangle = normalizedangle;
			drawblock.textureinfo1 = 0;
			drawblock.tex2 = 0;
			drawblock.tex3 = 0;
			drawblock.notexturerotate = 0;
		}
		drawport->XFillCircleEx(&drawblock);
	}
}

void setuparray(bool* ar, unsigned int size) {
	memset(ar, 0, size);
}

void XBallz::DisplayBallzFrame(XDrawPort* drawport, XTRect<> const* bounds, BallState* ballstate) {
	auto xballz = this;
	BallFrameEx* coords = (BallFrameEx*)xballz->GetCartesianCoordinates(ballstate);
	auto localbounds = XTRect<>{
		0,0,0,0
	};
	auto lnz = xballz->linez;
	auto boundspoint = XTPoint<int>{};
	auto circlerenderblock = CircleRenderBlock(lnz->rendermode);
	bool linerenderedarray[512]{ 0 };
	setuparray(xballz->renderedarray, 512);
	if (bounds->x1 < bounds->x2 && bounds->y1 < bounds->y2) {
		auto localdrawport = XDrawPort();
		if (ballstate->doclip) {
			localbounds.x2 = bounds->x2 - bounds->x1;
			localbounds.y2 = bounds->y2 - bounds->y1;
			localdrawport.XInitPort(&localbounds, 8, false, true, false);
			localdrawport.SetOrigin(-bounds->x1, -bounds->y1);
			drawport->XCopyBits(&localdrawport, bounds, bounds, (BrushType)0);
		}
		boundspoint.x = bounds->x1 - coords->x1;
		boundspoint.y = bounds->y1 - coords->y1;
		if (xballz->zorderflag == false) {
			xballz->ZOrder(coords, 4);
		}
		else {
			xballz->ZOrder(coords, 0);
			xballz->zorderflag = false;
		}
		auto total = xballz->addballcount + xballz->numballz;
		bool clipped = false;
		bool drewstacked = false;
		if (total > 0) {
			auto ballnoptr = xballz->ballnoarray;

			for (int cur = 0; cur < total; cur++) {
				int theballno = ballnoptr[cur];
				//if (some body area check here - didn't implement it yet)
				{
					XSprite* othersprite = (XSprite*)ballstate->unknownptr[6];
					if (!drewstacked) {
						drewstacked = true;
						if (othersprite != nullptr) {
							othersprite->DrawStacked(drawport, (EStackDraw)1);
						}
					}
					if (lnz->unknown2 == cur && othersprite != nullptr) {
						othersprite->DrawStacked(drawport, (EStackDraw)2);
					}
				}
				if (theballno < xballz->numballz || 0 < this->addballarray[theballno - xballz->numballz]) {
					bool isEyeball = false;
					if (theballno == lnz->eye1ballno || theballno == lnz->eye2ballno) {
						isEyeball = true;
					}
					if (theballno != lnz->iris1ballno && theballno != lnz->iris2ballno) {
						if (lnz->omissions[theballno] == false) {
							auto ballsize = coords->ballsize[theballno];
							if (0 < ballsize && lnz->bodyareas[theballno] != 14) {
								if (ballstate->doclip && !clipped && coords->posrotinfo[theballno].z < ballstate->someclipinfo + ballstate->someclipinfo2) {
									clipped = true;
									xballz->Clip(*ballstate, *bounds, localdrawport, *drawport);
								}
								auto localpoint2 = XTPoint<int>{
									coords->posrotinfo[theballno].x + boundspoint.x,
									coords->posrotinfo[theballno].y + boundspoint.y
								};
								circlerenderblock.rect.x1 = localpoint2.x - ballsize;
								circlerenderblock.rect.x2 = localpoint2.x + ballsize;
								circlerenderblock.rect.y1 = localpoint2.y - ballsize;
								circlerenderblock.rect.y2 = localpoint2.y + ballsize;
								if (lnz->linezinfo2[theballno].start > 0) {
									xballz->DrawAllLines(drawport, ballstate, coords, (pfarray<bool, 512>&)linerenderedarray, theballno, boundspoint);
								}
								int eyesize = ballstate->eyeballsize;
								if (eyesize != 0 && isEyeball) {
									circlerenderblock.rect.x1 -= eyesize;
									circlerenderblock.rect.y1 -= eyesize;
									circlerenderblock.rect.x2 += eyesize;
									circlerenderblock.rect.y2 += eyesize;
								}
								circlerenderblock.outlinetype = lnz->outlinearray[theballno];
								int colorindex = lnz->colorindexarray[theballno];
								if (!isEyeball) {
									if (9 < colorindex && colorindex < 150) {
										signed int colorindexmiddle = (colorindex - 10) / 10;
										int zscale = colorindex + xballz->zscale[theballno];
										int min = (colorindexmiddle * 5 + 5) * 2;
										int max = colorindexmiddle * 10 + 19;
										int actualcolor = min(max, max(zscale, min));
										colorindex = actualcolor;
									}
								}
								circlerenderblock.colorindex = colorindex;
								circlerenderblock.outlinecolor = lnz->outlinecolorarray[theballno];
								circlerenderblock.fuzz = lnz->fuzz[theballno] + lnz->fuzzoverride[theballno];
								xballz->SetBallTextureInfo(circlerenderblock, *ballstate, coords->posrotinfo, theballno);
								auto disabletextures = g_ShlGlobals->vars[4];
								if (disabletextures) {
									circlerenderblock.textureinfo1 = 0;
								}
								auto result = drawport->XFillCircleEx(&circlerenderblock);
								if (result) {
									if (0 < (&lnz->somestruct)[theballno * 5 + 570]) {
										xballz->DrawPaintBallz(*drawport, theballno, circlerenderblock, *ballstate, coords->posrotinfo);
									}
									if (isEyeball) {
										//DrawEyeball(drawport, coords, ballstate, theballno, circlerenderblock, ballsize, localpoint2);
										patch::DrawEyeball(xballz, drawport, coords, ballstate, theballno, circlerenderblock, ballsize, localpoint2);
									}
									if ((&lnz->somestruct)[theballno * 3 + 16437] > 0) {
										xballz->DrawAllWhiskers(drawport, *ballstate, coords, theballno, boundspoint);
									}
								}
								xballz->renderedarray[theballno] = true;
								XSprite* othersprite = *(XSprite**)(*ballstate->unknownptr + 12 + theballno * 16);
								if (theballno < ballstate->unknownptr[1] && othersprite != 0) {
									othersprite->DrawStacked(drawport, (EStackDraw)0);
								}
								if (lnz->linezinfo2[theballno].option2 != 0 && lnz->linezinfo2[theballno].start > 0) {
									xballz->DrawAllLines(drawport, ballstate, coords, (pfarray<bool, 512>&)linerenderedarray, theballno, boundspoint);
								}
							}
						}
						else {
							if (theballno < ballstate->unknownptr[1]) {
								XSprite* othersprite = *(XSprite**)(*ballstate->unknownptr + 12 + theballno * 16);
								if (othersprite != 0) {
									othersprite->DrawStacked(drawport, (EStackDraw)0);
								}
							}
						}
					}
				}
			}
		}
		if (ballstate->doclip && !clipped) {
			xballz->Clip(*ballstate, *bounds, localdrawport, *drawport);
		}
	}

}