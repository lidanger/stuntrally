#include "pch.h"
#include "common/Defines.h"
#include "CGame.h"
#include "CHud.h"
#include "CGui.h"
#include "../vdrift/game.h"
#include "../road/Road.h"
#include "SplitScreen.h"
#include "common/RenderConst.h"

#include <OgreRenderWindow.h>
#include <OgreSceneNode.h>
#include <OgreMaterialManager.h>
#include <OgreManualObject.h>
#include <OgrePass.h>
#include <OgreTechnique.h>
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreOverlayManager.h>
#include <OgreOverlayElement.h>
using namespace Ogre;
using namespace MyGUI;


///  HUD resize
//---------------------------------------------------------------------------------------------------------------
void CHud::Size(bool full, Viewport* vp)
{
	float wx = app->mWindow->getWidth(), wy = app->mWindow->getHeight();
	asp = wx/wy;
	bool vdrSpl = app->sc->vdr && pSet->game.local_players > 1;
	int plr = (int)app->carModels.size() -(app->isGhost2nd?1:0);  // others

	int cnt = pSet->game.local_players;
	#ifdef DEBUG
	assert(cnt <= hud.size());
	#endif
	//  for each car
	for (int c=0; c < cnt; ++c)
	{
		Hud& h = hud[c];
		const SplitScreenManager::VPDims& dim = app->mSplitMgr->mDims[c];
		//  gauges
		Real xcRpm,ycRpm, xcVel,ycVel, ygMax, xBFuel;  // -1..1
		if (h.ndGauges)
		{
			Real sc = pSet->size_gauges * dim.avgsize;
			Real spx = sc * 1.1f, spy = spx*asp;
			//xcRpm = dim.left + spx;   ycRpm =-dim.bottom + spy;
			xcRpm = dim.right - spx*0.5f;  ycRpm =-dim.bottom + spy*2.f;
			xcVel = dim.right - spx;       ycVel =-dim.bottom + spy*0.9f;
			ygMax = ycVel - sc;  xBFuel = xcVel - sc;

			h.vcRpm = Vector2(xcRpm,ycRpm);  // store for updates
			h.vcVel = Vector2(xcVel,ycVel);
			h.fScale = sc;
			h.updGauges = true;
		}
		//  minimap
		Real sc = pSet->size_minimap * dim.avgsize;
		const Real marg = 1.3f; //1.05f;  // from border
		Real fMiniX = vdrSpl ? (1.f - 2.f*sc * marg) : (dim.left + sc * marg);  //(dim.right - sc * marg);
		Real fMiniY =-dim.bottom + sc*asp * marg;  //-dim.top - sc*asp * marg;
		Real miniTopY = fMiniY + sc*asp * 1.5f;  //par above

		if (h.ndMap)
		{
			h.ndMap->setScale((vdrSpl ? 2 : 1)*sc, sc*asp,1);
			h.ndMap->setPosition(Vector3(fMiniX,fMiniY,0.f));
		}
	
		//  current viewport max x,y in pixels
		int xMin = (dim.left+1.f)*0.5f*wx, xMax = (dim.right +1.f)*0.5f*wx,
			yMin = (dim.top +1.f)*0.5f*wy, yMax = (dim.bottom+1.f)*0.5f*wy;
		int my = (1.f-ygMax)*0.5f*wy;  // gauge bottom y

		//  gear, vel
		//  positioning, min yMax - dont go below viewport bottom
		if (h.txGear)
		{
			int vv = pSet->gauges_type > 0 ? -45 : 40;
			int gx = (xcRpm+1.f)*0.5f*wx - 10, gy = (-ycRpm+1.f)*0.5f*wy +22;
			int vx = (xcVel+1.f)*0.5f*wx + vv, vy = std::min(yMax -91, my - 15);
			int bx =(xBFuel+1.f)*0.5f*wx - 10, by = std::min(yMax -36, my + 5);
				vx = std::min(vx, xMax -100);
				bx = std::min(bx, xMax -180);  // not too near to vel
			h.txGear->setPosition(gx,gy);
			h.txVel->setPosition(vx,vy);  //h.bckVel

			#if 0
			h.txRewind ->setPosition(bx,   by);
			h.icoRewind->setPosition(bx+50,by-5);
			#endif

			h.txDamage ->setPosition(bx-70,   by-70);
			h.icoDamage->setPosition(bx-70+50,by-70-5);

			h.txBFuel ->setPosition(bx-63,   by-140);
			h.icoBFuel->setPosition(bx-63+54,by-140-5+2);

			//  times
			bool hasLaps = pSet->game.local_players > 1 || pSet->game.champ_num >= 0 || app->mClient;
			int tx = xMin + 20, ty = yMin + 40;  // above minimap
			h.bckTimes->setPosition(tx,ty);
			tx = 24;  ty = 4;  //(hasLaps ? 16 : 4);
			h.txTimTxt->setPosition(tx,ty);
			h.txTimes->setPosition(tx+126,ty);
				
			//  opp list
			//int ox = itx + 5, oy = (ycRpm+1.f)*0.5f*wy - 10;
			int ox = xMin + 50, oy = (-miniTopY+1.f)*0.5f*wy;  //ty + 440;
			h.bckOpp->setPosition(ox,oy -2);  h.bckOpp->setSize(230, plr*25 +4);
			for (int n=0; n<3; ++n)
				h.txOpp[n]->setPosition(n*65+5,0);
			
			//  warn,win
			ox = xMin + 300;  oy = yMin + 15;
			h.bckWarn->setPosition(ox,oy);
			h.bckPlace->setPosition(ox,oy + 40);
			
			h.txCountdown->setPosition((xMax-xMin)/2 -100, (yMax-yMin)/2 -60);
			//  camera
			h.txCam->setPosition(xMax-260,yMax-30);
			//  abs,tcs
			h.txAbs->setPosition(xMin+160,yMax-30);
			h.txTcs->setPosition(xMin+220,yMax-30);
		}
	}
	if (txCamInfo)
	{	txCamInfo->setPosition(300,wy-100);
		bckMsg->setPosition(400,10);
	}
}

///---------------------------------------------------------------------------------------------------------------
///  HUD create
///---------------------------------------------------------------------------------------------------------------

void CHud::Create()
{
	//Destroy();  //
	if (app->carModels.size() == 0)  return;

	QTimer ti;  ti.update();  /// time

	SceneManager* scm = app->mSplitMgr->mGuiSceneMgr;
	if (hud[0].moMap || hud[0].txVel || hud[0].bckTimes)
		LogO("CreateHUD: Hud exists !");

	app->CreateGraphs();
		
	//  minimap from road img
	int plr = app->mSplitMgr->mNumViewports;  // pSet->game.local_players;
	LogO("-- Create Hud  plrs="+toStr(plr));
	asp = 1.f;

	///  reload mini textures
	ResourceGroupManager& resMgr = ResourceGroupManager::getSingleton();
	Ogre::TextureManager& texMgr = Ogre::TextureManager::getSingleton();

	String path = app->bRplPlay ? 
		gui->PathListTrkPrv(app->replay.header.track_user, app->replay.header.track) :
		gui->PathListTrkPrv(pSet->game.track_user, pSet->game.track);
	const String sRoad = "road.png", sTer = "terrain.jpg", sGrp = "TrkMini";
	resMgr.addResourceLocation(path, "FileSystem", sGrp);  // add for this track
	resMgr.unloadResourceGroup(sGrp);
	resMgr.initialiseResourceGroup(sGrp);

	if (app->sc->ter)
	{	try {  texMgr.unload(sRoad);  texMgr.load(sRoad, sGrp, TEX_TYPE_2D, MIP_UNLIMITED);  }  catch(...) {  }
		try {  texMgr.unload(sTer);   texMgr.load(sTer,  sGrp, TEX_TYPE_2D, MIP_UNLIMITED);  }  catch(...) {  }
	}

	//if (terrain)
	int cnt = std::min(6/**/, (int)app->carModels.size() -(app->isGhost2nd?1:0) );  // others
	#ifdef DEBUG
	assert(plr <= hud.size());
	assert(cnt <= hud[0].vMoPos.size());
	#endif
	int y=1200; //off 0
	
	//  for each car
	for (int c=0; c < plr; ++c)
	{
		String s = toStr(c);
		Hud& h = hud[c];
		if (app->sc->ter)
		{	float t = app->sc->td.fTerWorldSize*0.5;
			minX = -t;  minY = -t;  maxX = t;  maxY = t;  }

		float fMapSizeX = maxX - minX, fMapSizeY = maxY - minY;  // map size
		float size = std::max(fMapSizeX, fMapSizeY*asp);
		scX = 1.f / size;  scY = 1.f / size;

		String sMat = "circle_minimap";
		asp = 1.f;  //_temp

		ManualObject* m = Create2D(sMat,scm,1, true,true, 1.f,Vector2(1,1), RV_Hud,RQG_Hud1);  h.moMap = m;
		//asp = float(mWindow->getWidth())/float(mWindow->getHeight());
		
		//  change minimap image
		MaterialPtr mm = MaterialManager::getSingleton().getByName(sMat);
		Pass* pass = mm->getTechnique(0)->getPass(0);
		TextureUnitState* tus = pass->getTextureUnitState(0);
		if (tus)  tus->setTextureName(app->sc->ter ? sRoad : "alpha.png");
		tus = pass->getTextureUnitState(2);
		if (tus)  tus->setTextureName(app->sc->ter ? sTer : "alpha.png");
		UpdMiniTer();
		
		float fHudSize = pSet->size_minimap * app->mSplitMgr->mDims[c].avgsize;
		SceneNode* rt = scm->getRootSceneNode();
		if (!app->sc->vdr)
		{	h.ndMap = rt->createChildSceneNode(Vector3(0,0,0));
			h.ndMap->attachObject(m);
		}
		//  car pos tri - for all carModels (ghost and remote too)
		for (int i=0; i < cnt; ++i)
		{
			h.vMoPos[i] = Create2D("hud/CarPos", scm, 0.4f, true,true, 1.f,Vector2(1,1), RV_Hud,RQG_Hud3);
				  
			h.vNdPos[i] = h.ndMap ? h.ndMap->createChildSceneNode() : hud[0].ndMap->createChildSceneNode();
			h.vNdPos[i]->scale(fHudSize*1.5f, fHudSize*1.5f, 1);
			h.vNdPos[i]->attachObject(h.vMoPos[i]);
		}
		if (h.ndMap)
			h.ndMap->setVisible(false/*pSet->trackmap*/);

	
		//  gauges  backgr  -----------
		String st = toStr(pSet->gauges_type);
		const Real sc = 0.5f;
		h.moGauges = Create2D("hud/"+st,scm,1, true,false, sc,Vector2(0.f,0.5f), RV_Hud,RQG_Hud1, true);
		h.ndGauges = rt->createChildSceneNode();  h.ndGauges->attachObject(h.moGauges);  h.ndGauges->setVisible(false);

		//  gauges  needles
		h.moNeedles = Create2D("hud/"+st,scm,1, true,false, sc,Vector2(0.5f,0.5f), RV_Hud,RQG_Hud3, true);
		h.ndNeedles = rt->createChildSceneNode();  h.ndNeedles->attachObject(h.moNeedles);  h.ndNeedles->setVisible(false);


		///  GUI
		//  gear, vel text  -----------
		h.parent = app->mGUI->createWidget<Widget>("",0,0,2560,1600,Align::Left,"Back","main"+s);

		h.txGear = h.parent->createWidget<TextBox>("TextBox",
			0,y, 160,116, Align::Left, "Gear"+s);  h.txGear->setVisible(false);
		h.txGear->setFontName("DigGear");  h.txGear->setFontHeight(126);
		//h.txGear->setTextShadow(true);

		/*h.bckVel = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 130,82, Align::Left, "IVel"+s);
		h.bckVel->setImageTexture("back_vel.png");
		h.bckVel->setAlpha(0.7f);*/
		
		//h.txVel = h.bckVel->createWidget<TextBox>("TextBox",
		//	10,5, 360,96, Align::Right, "Vel"+s);  h.txVel->setVisible(false);
		h.txVel = h.parent->createWidget<TextBox>("TextBox",
			0,y, 360,96, Align::Right, "Vel"+s);  h.txVel->setVisible(false);
		h.txVel->setFontName("DigGear");  //h.txVel->setFontHeight(64);
		//h.txVel->setInheritsAlpha(false);
		//h.txVel->setTextShadow(true);
		
		//  boost
		h.txBFuel = h.parent->createWidget<TextBox>("TextBox",
			0,y, 240,80, Align::Right, "Fuel"+s);  h.txBFuel->setVisible(false);
		h.txBFuel->setFontName("DigGear");  h.txBFuel->setFontHeight(72);
		h.txBFuel->setTextColour(Colour(0.6,0.8,1.0));  //h.txBFuel->setTextShadow(true);

		h.icoBFuel = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 40,40, Align::Left, "IFuel"+s);  //h.icoBFuel->setVisible(false);
		h.icoBFuel->setImageTexture("gui_icons.png");
		h.icoBFuel->setImageCoord(IntCoord(512,0,128,128));

		//  damage %
		if (pSet->game.damage_type > 0)
		{
			h.txDamage = h.parent->createWidget<TextBox>("TextBox",
				0,y, 240,80, Align::Right, "Dmg"+s);  //h.txDamage->setVisible(false);
			h.txDamage->setFontName("font.24");  //h.txDamage->setFontHeight(64);
			h.txDamage->setTextColour(Colour(0.7,0.7,0.7));  h.txDamage->setTextShadow(true);

			h.icoDamage = h.parent->createWidget<ImageBox>("ImageBox",
				0,y, 40,40, Align::Left, "IDmg"+s);  //h.icoDamage->setVisible(false);
			h.icoDamage->setImageTexture("gui_icons.png");
			h.icoDamage->setImageCoord(IntCoord(512,256,128,128));
		}
		
		//  rewind <<
		#if 0
		h.txRewind = h.parent->createWidget<TextBox>("TextBox",
			0,y, 240,80, Align::Right, "Rew"+s);  //h.txRewind->setVisible(false);
		h.txRewind->setFontName("DigGear");  h.txRewind->setFontHeight(64);
		h.txRewind->setTextColour(Colour(0.9,0.7,1.0));
		h.txRewind->setCaption("3.0");

		h.icoRewind = h.parent->createWidget<ImageBox>("ImageBox",
			200,180, 40,40, Align::Left, "IRew"+s);  //h.icoRewind->setVisible(false);
		h.icoRewind->setImageTexture("gui_icons.png");
		h.icoRewind->setImageCoord(IntCoord(512,384,128,128));
		#endif


		//  times text  -----------
		h.bckTimes = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 356,260, Align::Left, "TimP"+s);  h.bckTimes->setVisible(false);
		h.bckTimes->setAlpha(0.1f);
		h.bckTimes->setImageTexture("back_times.png");

		h.txTimTxt = h.bckTimes->createWidget<TextBox>("TextBox",
			0,y, 120,260, Align::Left, "TimT"+s);
		h.txTimTxt->setFontName("font.22");  h.txTimTxt->setTextShadow(true);
		h.txTimTxt->setInheritsAlpha(false);
		bool hasLaps = pSet->game.local_players > 1 || pSet->game.champ_num >= 0 || pSet->game.chall_num >= 0 || app->mClient;
		h.txTimTxt->setCaption(
			(hasLaps ? String("#D0F8F0")+TR("#{TBLap}") : "")+
			"\n#C0E0F0"+TR("#{TBTime}") + 
			"\n#80C0F0"+TR("#{TBLast}") + 
			"\n#80E0E0"+TR("#{TBBest}") +
			"\n#70D070"+TR("#{Track}") +
			"\n\n#C0C030"+TR("#{TBPosition}") +
			"\n#F0C050"+TR("#{TBPoints}") );

		h.txTimes = h.bckTimes->createWidget<TextBox>("TextBox",
			0,y, 230,260, Align::Left, "Tim"+s);
		h.txTimes->setInheritsAlpha(false);
		h.txTimes->setFontName("font.22");  h.txTimes->setTextShadow(true);


		//  opp list  -----------
		h.bckOpp = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 224,200, Align::Left, "OppB"+toStr(c));
		h.bckOpp->setAlpha(0.9f);  h.bckOpp->setVisible(false);
		h.bckOpp->setImageTexture("opp_rect.png");

		for (int n=0; n < 3; ++n)
		{
			h.txOpp[n] = h.bckOpp->createWidget<TextBox>("TextBox",
				n*80+10,0, 90,180, n == 2 ? Align::Left : Align::Right, "Opp"+toStr(n)+s);
			h.txOpp[n]->setFontName("font.20");
			if (n==0)  h.txOpp[n]->setTextShadow(true);
		}

		//  wrong chk warning  -----------
		h.bckWarn = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 400,60, Align::Left, "WarnB"+s);  h.bckWarn->setVisible(false);
		h.bckWarn->setImageTexture("back_times.png");

		h.txWarn = h.bckWarn->createWidget<TextBox>("TextBox",
			0,0, 400,60, Align::Left, "WarnT"+s);
		h.txWarn->setFontName("font.20");  h.txWarn->setTextShadow(true);
		h.txWarn->setTextColour(Colour(1,0.3,0));  h.txWarn->setTextAlign(Align::Center);
		h.txWarn->setCaption(TR("#{WrongChk}"));

		//  win place  -----------
		h.bckPlace = h.parent->createWidget<ImageBox>("ImageBox",
			0,y, 400,60, Align::Left, "PlcB"+s);  h.bckPlace->setVisible(false);
		h.bckPlace->setImageTexture("back_times.png");

		h.txPlace = h.bckPlace->createWidget<TextBox>("TextBox",
			0,0, 400,60, Align::Left, "PlcT"+s);
		h.txPlace->setFontName("font.20");  h.txPlace->setTextShadow(true);
		h.txPlace->setTextAlign(Align::Center);

		//  start countdown
		h.txCountdown = h.parent->createWidget<TextBox>("TextBox",
			0,y, 200,120, Align::Left, "CntT"+s);  h.txCountdown->setVisible(false);
		h.txCountdown->setFontName("DigGear");  h.txCountdown->setTextShadow(true);
		h.txCountdown->setTextColour(Colour(0.8,0.9,1));  h.txCountdown->setTextAlign(Align::Center);

		//  abs, tcs
		h.txAbs = h.parent->createWidget<TextBox>("TextBox",
			0,y, 120,60, Align::Left, "AbsT"+s);
		h.txAbs->setFontName("font.19");  h.txAbs->setTextShadow(true);
		h.txAbs->setCaption("ABS");  h.txAbs->setTextColour(Colour(1,1,0.6));

		h.txTcs = h.parent->createWidget<TextBox>("TextBox",
			0,y, 120,60, Align::Left, "TcsT"+s);
		h.txTcs->setFontName("font.19");  h.txTcs->setTextShadow(true);
		h.txTcs->setCaption("TCS");  h.txTcs->setTextColour(Colour(0.6,1,1));

		//  camera name
		h.txCam = h.parent->createWidget<TextBox>("TextBox",
			0,0, 200,30, Align::Left, "CamT"+s);
		h.txCam->setFontName("font.20");  h.txCam->setTextShadow(true);
		h.txCam->setTextColour(Colour(0.65,0.85,0.85));
	}

	//  camera info
	txCamInfo = app->mGUI->createWidget<TextBox>("TextBox",
		0,y, 800,100, Align::Left, "Back", "CamIT");  txCamInfo->setVisible(false);
	txCamInfo->setFontName("font.20");  txCamInfo->setTextShadow(true);
	txCamInfo->setTextColour(Colour(0.8,0.9,0.9));

	//  chat msg  -----------
	bckMsg = app->mGUI->createWidget<ImageBox>("ImageBox",
		0,y, 400,60, Align::Left, "Back", "MsgB");  bckMsg->setVisible(false);
	bckMsg->setAlpha(0.8f);
	bckMsg->setImageTexture("back_times.png");

	txMsg = bckMsg->createWidget<TextBox>("TextBox",
		10,10, 800,60, Align::Left, "PlcT");
	txMsg->setFontName("font.20");  txMsg->setTextShadow(true);
	txMsg->setTextColour(Colour(0.8,0.9,1.0));

	///  tex
	resMgr.removeResourceLocation(path, sGrp);
	
	//-  cars need update
	for (int i=0; i < app->carModels.size(); ++i)
		app->carModels[i]->updTimes = true;

	
	///  tire vis circles  + + + +
	asp = float(app->mWindow->getWidth())/float(app->mWindow->getHeight());

	if (pSet->car_tirevis)
	{	SceneNode* rt = scm->getRootSceneNode();
		for (int i=0; i < 4; ++i)
		{
			ManualObject* m = app->mSceneMgr->createManualObject();
			m->setDynamic(true);
			m->setUseIdentityProjection(true);
			m->setUseIdentityView(true);
			m->setCastShadows(false);

			m->estimateVertexCount(32);
			m->begin("hud/line", RenderOperation::OT_LINE_LIST);
			m->position(-1,0, 0);  m->colour(1,1,1);
			m->position( 1,0, 0);  m->colour(1,1,1);
			m->end();
		 
			AxisAlignedBox aabInf;	aabInf.setInfinite();
			m->setBoundingBox(aabInf);  // always visible
			m->setVisibilityFlags(RV_Hud);
			m->setRenderQueueGroup(RQG_Hud1);

			moTireVis[i] = m;
			ndTireVis[i] = rt->createChildSceneNode();  ndTireVis[i]->attachObject(moTireVis[i]);
			ndTireVis[i]->setPosition((i%2 ? 1.f :-1.f) * 0.14f - 0.7f,
									  (i/2 ?-1.f : 1.f) * 0.22f - 0.5f, 0.f);
			const Real s = 0.06f;  // par
			ndTireVis[i]->setScale(s, s*asp, 1.f);
	}	}


	//  dbg texts
	OverlayManager& ovr = OverlayManager::getSingleton();
	ovCarDbg = ovr.getByName("Car/Stats");
	ovCarDbgTxt = ovr.getByName("Car/StatsTxt");
	ovCarDbgExt = ovr.getByName("Car/StatsExt");
	
	for (int i=0; i < ov.size(); ++i)
	{	String s = toStr(i+1);
		ov[i].oL = ovr.getOverlayElement("L_"+s);	ov[i].oR = ovr.getOverlayElement("R_"+s);
		ov[i].oS = ovr.getOverlayElement("S_"+s);	ov[i].oU = ovr.getOverlayElement("U_"+s);
		ov[i].oX = ovr.getOverlayElement("X_"+s);
	}
	Show();  //_
	app->bSizeHUD = true;
	//SizeHUD(true);
	
	ti.update();	/// time
	float dt = ti.dt * 1000.f;
	LogO("::: Time Create Hud: "+fToStr(dt,0,3)+" ms");
}



//  HUD destroy
//---------------------------------------------------------------------------------------------------------------
CHud::OvrDbg::OvrDbg() :
	oL(0),oR(0),oS(0), oU(0),oX(0)
{	}

CHud::Hud::Hud()
	:parent(0)
	,txTimTxt(0), txTimes(0), bckTimes(0),  sTimes("")
	,bckOpp(0)
	,txWarn(0), txPlace(0),  bckWarn(0), bckPlace(0)
	,txCountdown(0)

	,txGear(0), txVel(0), bckVel(0)
	,ndNeedles(0), ndGauges(0)
	,moNeedles(0), moGauges(0)
	,txAbs(0), txTcs(0),  txCam(0)

	,txBFuel(0), txDamage(0), txRewind(0)
	,icoBFuel(0), icoDamage(0), icoRewind(0)

	,moMap(0),  ndMap(0)
{
	for (int i=0; i<3; ++i)  txOpp[i]=0;
	vNdPos.resize(6,0);  vMoPos.resize(6,0);
}

void CHud::Destroy()
{
	SceneManager* scm = app->mSplitMgr->mGuiSceneMgr;
	int i,c;
	for (c=0; c < hud.size(); ++c)
	{	Hud& h = hud[c];

		#define Dest2(mo,nd)  {  \
			if (mo) {  scm->destroyManualObject(mo);  mo=0;  } \
			if (nd) {  scm->destroySceneNode(nd);  nd=0;  }  }

		for (i=0; i < 6; ++i)
			Dest2(h.vMoPos[i],h.vNdPos[i])
		
		Dest2(h.moMap,h.ndMap)
		Dest2(h.moGauges,h.ndGauges)
		Dest2(h.moNeedles,h.ndNeedles)

		#define Dest(w)  \
			if (w) {  app->mGUI->destroyWidget(w);  w = 0;  }
		Dest(h.txGear)  Dest(h.txVel)  Dest(h.bckVel)
		Dest(h.txAbs)  Dest(h.txTcs)  Dest(h.txCam)
		
		Dest(h.txBFuel)  Dest(h.txDamage)  Dest(h.txRewind)
		Dest(h.icoBFuel)  Dest(h.icoDamage)  Dest(h.icoRewind)

		for (i=0; i < 3; ++i)  Dest(h.txOpp[i])
		Dest(h.bckOpp)
		Dest(h.txTimTxt)  Dest(h.txTimes)  Dest(h.bckTimes)
		h.sTimes = "";
		
		Dest(h.txWarn)  Dest(h.bckWarn)
		Dest(h.txPlace)  Dest(h.bckPlace)
		Dest(h.txCountdown)
	}
	Dest(txMsg)  Dest(bckMsg)
	Dest(txCamInfo)
	
	for (i=0; i < 4; ++i)
		Dest2(moTireVis[i],ndTireVis[i])
}

//  HUD show/hide
//---------------------------------------------------------------------------------------------------------------
void CHud::Show(bool hideAll)
{
	if (hideAll || app->iLoad1stFrames >= 0)  // still loading
	{
		if (ovCarDbg)  ovCarDbg->hide();
		if (ovCarDbgTxt)  ovCarDbgTxt->hide();

		app->bckFps->setVisible(false);
		if (bckMsg)
		{
			txCamInfo->setVisible(false);
			bckMsg->setVisible(false);

			for (int c=0; c < hud.size(); ++c)
			{	Hud& h = hud[c];
				if (h.parent)
					h.parent->setVisible(false);
		}	}
		app->hideMouse();
		if (app->mWndRpl)  app->mWndRpl->setVisible(false);
		return;
	}
	//  this goes each frame..
	bool show = pSet->car_dbgbars;
	if (ovCarDbg){  if (show)  ovCarDbg->show();  else  ovCarDbg->hide();  }
	show = pSet->car_dbgtxt || pSet->bltProfilerTxt || pSet->profilerTxt;
	if (ovCarDbgTxt){  if (show)  ovCarDbgTxt->show();  else  ovCarDbgTxt->hide();  }
	show = pSet->car_dbgsurf;
	if (ovCarDbgExt){  if (show)  ovCarDbgExt->show();  else  ovCarDbgExt->hide();  }

	app->bckFps->setVisible(pSet->show_fps);
	if (bckMsg)
	{
		bool cam = pSet->show_cam && !app->isFocGui, times = pSet->show_times;
		bool opp = pSet->show_opponents && (!app->sc->ter || app->road && app->road->getNumPoints() > 0);
		bool bfuel = pSet->game.boost_type == 1 || pSet->game.boost_type == 2;
		bool bdmg = pSet->game.damage_type > 0;
		txCamInfo->setVisible(cam);

		show = pSet->show_gauges;
		for (int c=0; c < hud.size(); ++c)
		{	Hud& h = hud[c];
			if (h.parent && h.txGear)
			{	h.parent->setVisible(true);
			
				h.txGear->setVisible(pSet->show_digits);
				h.txVel->setVisible(pSet->show_digits);
				h.txBFuel->setVisible(show && bfuel);  h.icoBFuel->setVisible(show && bfuel);
				h.txDamage->setVisible(show && bdmg);  h.icoDamage->setVisible(show && bdmg);
				//txRewind;icoRewind;

				h.ndGauges->setVisible(show);
				h.ndNeedles->setVisible(show);

				h.ndMap->setVisible(pSet->trackmap);
				h.bckTimes->setVisible(times);
				h.bckOpp->setVisible(opp);
				h.txCam->setVisible(cam);
		}	}
	}
	app->updMouse();
	if (app->mWndRpl && !app->bLoading)  app->mWndRpl->setVisible(app->bRplPlay && app->bRplWnd);  //
}

void CHud::ShowVp(bool vp)	// todo: use vis mask ..
{
	// show/hide for render viewport / gui viewport
	// first show everything
	Show(false);  // todo: don't here
	// now hide things we dont want
	if (!vp)
	{
		/// for gui viewport ----------------------
		if (ovCarDbg)  ovCarDbg->hide();	if (ovCarDbgTxt)  ovCarDbgTxt->hide();
	}else{
		/// for render viewport ---------
		//if (ovCam)  ovCam->hide();
		//bckFps->setVisible(false);
	}
}


void CHud::CreateArrow()
{
	if (!arrow.node)  arrow.node = app->mSceneMgr->getRootSceneNode()->createChildSceneNode();
	Ogre::Entity* ent = app->mSceneMgr->createEntity("CheckpointArrow", "arrow.mesh");
	ent->setRenderQueueGroup(RQG_Hud3);
	ent->setCastShadows(false);
	arrow.nodeRot = arrow.node->createChildSceneNode();
	arrow.nodeRot->attachObject(ent);
	arrow.nodeRot->setScale(pSet->size_arrow/2.f, pSet->size_arrow/2.f, pSet->size_arrow/2.f);
	ent->setVisibilityFlags(RV_Hud);
	arrow.nodeRot->setVisible(pSet->check_arrow);
}
