#include "Role.h"
#include "MainRoot.h"
#include "World.h"
#include "Audio.h"
#include "PlayerMe.h"
#include "RPGSkyUIGraph.h"
#include "CsvFile.h"
#include "SkinMesh.h"
#include "SkeletonData.h"
#include "Res.h"

//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

const int DX[DIR_COUNT] = { 0, 1, 1, 1, 0,-1,-1,-1};
const int DY[DIR_COUNT] = {-1,-1, 0, 1, 1, 1, 0,-1};
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

const Vec3D DirNormalize[DIR_COUNT] = 
{
	DIR_NORMALIZE(0),
	DIR_NORMALIZE(1),
	DIR_NORMALIZE(2),
	DIR_NORMALIZE(3),
	DIR_NORMALIZE(4),
	DIR_NORMALIZE(5),
	DIR_NORMALIZE(6),
	DIR_NORMALIZE(7),
};
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

const float DirLength[DIR_COUNT] = 
{
	DIR_LENGTH(0),
	DIR_LENGTH(1),
	DIR_LENGTH(2),
	DIR_LENGTH(3),
	DIR_LENGTH(4),
	DIR_LENGTH(5),
	DIR_LENGTH(6),
	DIR_LENGTH(7),
};
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CRole::CRole()
	:m_fRoleHeight(0.0f)
{
	m_fHeadRotate 			= 0.0f;
	m_fCurrentHeadRotate 	= 0.0f;
	m_fWalkSpeed 			= 2.35f;
	// ----
	m_uActionState 			= STAND;
	m_uWeaponState			= WS_EMPTY;
	m_uSafeState			= 1;
	// ----
	m_uDir 					= 0;
	m_uTargetDir 			= 0;
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

CRole::~CRole()
{
	// ----
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::damage(int nDamage, unsigned char uDamageType, int nShieldDamage)
{
	std::wstring wcsDamageInfo;
	// ----
	if(nDamage==0)
	{
		wcsDamageInfo = L"Miss";
	}
	else
	{
		wchar_t wszTemp[255];
		if (CPlayerMe::getInstance().getID() == getID())
		{
			wcsDamageInfo = swprintf(wszTemp,L"[color=255,0,0]%d[/color]",nDamage);
		}
		else
		{
			wcsDamageInfo = swprintf(wszTemp,L"[color=255,96,0]%d[/color]",nDamage);
		}
	}
	// ----
	CWorld::getInstance().addDamageInfo(getPos(),wcsDamageInfo);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::setSkeleton()
{
	//BBox box;
	// ----
	// # Player.bmd
	// ----
	setFilename(PLAYER_BMD_PATCH);
	// ----
	//box.vMin = Vec3D(-0.5f, 0.0f, -0.5f);
	//box.vMax = Vec3D(0.5f, 2.0f, 0.5f);
	// ----
	//CModelObject::setBBox(box);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::setSet(const unsigned char* charSet)
{
	setClass(charSet[CS_CLASS]);
	// ----
	setSkeleton();
	// ----
	setEquip(ET_HELM,	GET_CHAR_H(charSet[3]));
	setEquip(ET_ARMOR,	GET_CHAR_L(charSet[3]));
	setEquip(ET_GLOVE,	GET_CHAR_H(charSet[4]));
	setEquip(ET_PANT,	GET_CHAR_L(charSet[4]));
	setEquip(ET_BOOT,	GET_CHAR_H(charSet[5]));
	setEquip(ET_BACK,	GET_CHAR_L(charSet[5]));
	setEquip(ET_RIGHT,	charSet[1]);
	setEquip(ET_LEFT,	charSet[2]);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::setEquipSkin(int nType, int nSkinID)
{
	if (nType<0 || nType>=ET_BACK)
	{
		return;
	}
	// ----
	CSkinMesh* pSkinMesh = (CSkinMesh*)getChild(EQUIP_TYPE_NAME[nType]);
	if (pSkinMesh)
	{
		pSkinMesh->setSkin(nSkinID);
	}
}

void CRole::setEquip(int nType, int nEquipID)
{
	if (nType<0 || nType>=ET_MAX)
	{
		return;
	}
	const char* szModelFilename	= NULL;
	switch (nType)
	{
	case ET_HELM:
	case ET_ARMOR:
	case ET_PANT:
	case ET_GLOVE:
	case ET_BOOT:
		{
			sprintf_s(m_szTemp, "Data\\Player\\%sClass%02d.bmd",EQUIP_TYPE_NAME[nType], (m_nClass >> 5) + 1);
			if(nEquipID < 0x0F)
			{
				szModelFilename	= CSVITEM.seek(0, nType+7).seek(1, nEquipID).getStr("Model","000000");
			}
			else
			{
				szModelFilename = m_szTemp;
			}
			// ----
			// # Delete old equip!
			// ----
			this->delChild(this->getChild(EQUIP_TYPE_NAME[nType]));
			// ----
			// # Attach new equip!
			// ----	
			CSkinMesh* pSkinMesh = new CSkinMesh;
			pSkinMesh->setFilename(szModelFilename);
			pSkinMesh->setName(EQUIP_TYPE_NAME[nType]);
			this->addChild(pSkinMesh);

			if (ET_HELM==nType)
			{
				this->delChild(this->getChild("face"));
				// ----
				if (CSVEQUIP.seek(0, nType+7).seek(1, nEquipID).getBool("ShowClass"))
				{
					CSkinMesh* pSkinMesh = new CSkinMesh;
					pSkinMesh->setFilename(m_szTemp);
					pSkinMesh->setName("face");
					this->addChild(pSkinMesh);
				}
			}
		}
		break;
		
	case ET_BACK:
		break;

	case ET_RIGHT:
		this->delChild(this->getChild(EQUIP_TYPE_NAME[nType]));
		if(nEquipID < 0xFF)
		{
			int nSeek1,nSeek2;
			if (m_nClass == 0x40)
			{
				if (nEquipID<7)
				{
					nSeek1 = 0;
					nSeek2 = nEquipID;
				}
				else
				{
					nSeek1 = 4;
					nSeek2 = nEquipID;
				}
			}
			else
			{
				nSeek1 = nEquipID>>6;
				nSeek2 = nEquipID;
			}
			// ----
			szModelFilename	= CSVITEM.seek(0, nSeek1).seek(1, nSeek2).getStr("Model","000000");
			// ----
			CSkeletonNode* pSkeleton = new CSkeletonNode;
			pSkeleton->setFilename(szModelFilename);
			pSkeleton->setName(EQUIP_TYPE_NAME[nType]);
			m_uWeaponState = WS_SINGLE;
			this->addChild(pSkeleton);
			updateAction();
		}
		break;

	case ET_LEFT:
		this->delChild(this->getChild(EQUIP_TYPE_NAME[nType]));
		if(nEquipID < 0xFF)
		{
			int nSeek1,nSeek2;
			if (m_nClass == 0x40)
			{
				if (nEquipID<7)
				{
					nSeek1 = 0;
					nSeek2 = nEquipID;
				}
				else
				{
					nSeek1 = 4;
					nSeek2 = nEquipID;
				}
			}
			else
			{
				nSeek1 = nEquipID>>6;
				nSeek2 = nEquipID;
			}
			// ----
			szModelFilename	= CSVITEM.seek(0, nSeek1).seek(1, nSeek2).getStr("Model","000000");
			// ----
			CSkeletonNode* pSkeleton = new CSkeletonNode;
			pSkeleton->setFilename(szModelFilename);
			pSkeleton->setName(EQUIP_TYPE_NAME[nType]);
			this->addChild(pSkeleton);
			updateAction();
		}
		break;
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::setCellPos(int x, int y)
{
	m_posCell.x 	= x;
	m_posCell.y 	= y;
	// ----
	m_vPos.x 		= (x + 0.5f);
	m_vPos.z 		= (y + 0.5f);
	// ----
	m_vPos.y		= CWorld::getInstance().getHeight(m_vPos.x, m_vPos.z);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::setTargetCellPos(int x, int y)
{
	m_uTargetDir = CWorld::getInstance().getPath(m_posCell.x, m_posCell.y, x, y, m_Path);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::drawName()const
{
	if(m_wstrName.empty() == false)
	{
		Pos2D pos(0,0);
		Vec3D vPos;
		// ----
		vPos 		= m_vPos;
		vPos.y 		+= m_fRoleHeight;
		// ----
		CRenderSystem & R = CRenderSystem::getSingleton();
		R.world2Screen(vPos, pos);
		// ----
		RECT rc 	={pos.x - 100, pos.y - 30 , pos.x + 100, pos.y};
		// ----
		//wchar_t wszTemp[255];
		//swprintf(wszTemp,L"[color=255,255,0]Level%d[/color] [color=128,255,255]%s[/color]",m_nLevel, m_wstrName.c_str());
		// ----
		R.SetShader("text");
		Node3DUIGraph::getInstance().drawText(m_wstrName.c_str(), m_wstrName.length(), rc, ALIGN_TYPE_CENTER);
	}
	//drawSkeleton(Matrix::newTranslation(Vec3D(1,0,0)), &((Node3DUIGraph*)&Node3DUIGraph::getInstance())->getTextRender());

}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::render(int nRenderType)const
{
	CSkeletonNode::render(nRenderType);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::animate(const Matrix& mWorld, double fTime, float fElapsedTime)
{
	// ----
	// # If death, do nothing.
	if(m_uActionState == DEATH)
	{
		return;
	}
	// ----
	CSkeletonNode::animate(mWorld,fTime,fElapsedTime);
	// ----
	// # Rotate Head
	if(m_pSkeletonData)
	{
		size_t uBoneCount = m_pSkeletonData->m_Bones.size();
		// ----
		for(size_t i = 0 ; i < uBoneCount; i++)
		{
			if(m_pSkeletonData->m_Bones[i].m_strName == "Bip01 Head")
			{
				if(rand() % 53 == 0)
				{
					m_fHeadRotate = (rand() % 100) * 0.02f - 1.0f;
				}
				// ----
				m_fCurrentHeadRotate += (m_fHeadRotate-m_fCurrentHeadRotate) * fElapsedTime;
				// ----
				Matrix mRotate;
				// ----
				mRotate.rotate(Vec3D(0.0f, m_fCurrentHeadRotate, 0.0f));
				// ----
				//m_setBonesMatrix[i] *= mRotate;
			}
		}
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void setWeaponAttach(CSkeletonNode* weapon, int id)
{
	CSVATTACH.seek(0, id);
	// ----
	weapon->setBindingBoneName(CSVATTACH.getStr("bone",0));
	weapon->setBindingBoneID(-1);
	Vec3D pos(CSVATTACH.getFloat("posx"),CSVATTACH.getFloat("posy"),CSVATTACH.getFloat("posz"));
	Vec3D rotate(CSVATTACH.getFloat("rotatex"),CSVATTACH.getFloat("rotatey"),CSVATTACH.getFloat("rotatez"));
	weapon->setPos(pos);
	weapon->setRotate(rotate);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::updateAction()
{
	if (m_uSafeState)
	{
		if (m_uActionState==RUN)
		{
			m_uActionState = WALK;
		}
		// ----
		CSkeletonNode* pWeapon1 = (CSkeletonNode*)getChild(EQUIP_TYPE_NAME[ET_RIGHT]);
		if (pWeapon1)
		{
			setWeaponAttach(pWeapon1,0);// 右背
		}
		// ----
		CSkeletonNode* pWeapon2 = (CSkeletonNode*)getChild(EQUIP_TYPE_NAME[ET_LEFT]);
		if (pWeapon2)
		{
			setWeaponAttach(pWeapon2,m_uWeaponState==WS_DSINGLE?2:1);// 盾:左背
		}
	}
	else
	{
		if (m_uActionState==WALK)
		{
			m_uActionState = RUN;
		}
		// ----
		CSkeletonNode* pWeapon1 = (CSkeletonNode*)getChild(EQUIP_TYPE_NAME[ET_RIGHT]);
		if (pWeapon1)
		{
			setWeaponAttach(pWeapon1,m_uWeaponState==WS_BOW?0:3);// 弓箭:右手
		}
		// ----
		CSkeletonNode* pWeapon2 = (CSkeletonNode*)getChild(EQUIP_TYPE_NAME[ET_LEFT]);
		if (pWeapon2)
		{
			setWeaponAttach(pWeapon2,m_uWeaponState==WS_CROSSBOW?1:m_uWeaponState==WS_DSINGLE?5:4);// 弩箭:盾:左手
		}
	}
	if (m_uActionState==RUN)
	{
		m_uActionState = WALK;
	}
	// ----
	int nAnim = CSVANIM.seek(0, 0).seek(1, -1).seek(2, m_uSafeState?WS_EMPTY:m_uWeaponState).getInt(m_uActionState+3,0);
	// ----
	if((m_nClass >> 5) == 2)
	{
		nAnim++;
	}
	// ----
	char szTemp[255];

	int uFrame = m_AnimMgr.uFrame;
	setAnimByName(itoa(nAnim,szTemp,10));
	if (m_uActionState != HIT1)
	{
		m_AnimMgr.uFrame = uFrame;
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::playWalkSound()
{
	// ----
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::moveStep()
{
	m_vPos.x	= (m_posCell.x + 0.5f);
	m_vPos.z 	= (m_posCell.y + 0.5f);
	m_vPos.y	= CWorld::getInstance().getHeight(m_vPos.x, m_vPos.z);
	// ----
	setDir(m_Path[0]);
	// ----
	m_posCell.x	+=DX[m_Path[0]];
	m_posCell.y	+=DY[m_Path[0]];
	// ----
	if (CWorld::getInstance().getSceneData())
	{
		unsigned char uSafeState = CWorld::getInstance().getSceneData()->getCellAttribute(m_posCell.x, m_posCell.y)&0x1;
		if(uSafeState!=m_uSafeState || (m_uActionState!=WALK&&m_uActionState!=RUN))
		{
			m_uSafeState = uSafeState;
			setActionState(WALK);
		}
	}
	// ----
	m_vNextPos.x = m_posCell.x + 0.5f;
	m_vNextPos.z = m_posCell.y + 0.5f;
	m_vNextPos.y = CWorld::getInstance().getHeight(m_vNextPos.x, m_vNextPos.z);
	// ----
	m_Path.pop_front();
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::frameMove(const Matrix& mWorld, double fTime, float fElapsedTime)
{
	// ----
	m_vRotate.x 			= 0.0f;
	m_vRotate.y 			= fmod(m_vRotate.y+PI_X2, PI_X2);
	// ----
	float fAngle 			= (m_uDir * PI * 0.25f - m_vRotate.y);
	fAngle 					+= (fAngle > PI ? - PI_X2 : (fAngle < - PI ? PI_X2 : 0));
	// ----
	m_vRotate.y 			+= fAngle * min(0.1f, fElapsedTime) * 10.0f;
	m_vRotate.z 			= 0.0f;
	// ----
	CSkeletonNode::frameMove(mWorld,fTime,fElapsedTime);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::frameMoveRole(const Matrix& mWorld, double fTime, float fElapsedTime)
{
	BBox box(-0.5f,0.0f,-0.5f, 0.5f,2.0f,0.5f);
	this->setLocalBBox(box);
	this->updateWorldBBox();
	this->updateWorldMatrix();
	if (m_fRoleHeight<=0.0f && m_pSkeletonData)
	{
		for (size_t i=0;i<m_pSkeletonData->m_Bones.size();++i)
		{
			Matrix	mInvLocal = m_pSkeletonData->m_Bones[i].m_mInvLocal;
			mInvLocal.Invert();
			Vec3D vBonesPoint = mInvLocal*Vec3D(0,0,0);
			if (m_fRoleHeight<vBonesPoint.y)
			{
				m_fRoleHeight = vBonesPoint.y;
			}
		}
	}
	if(m_uActionState == WALK || m_uActionState == RUN)
	{
		// Walk Speed = AnimWalkLength/(AnimTotalFrames*250ms/f)
		m_fWalkSpeed = m_uActionState == WALK?12.2f:13.4f;
		float fWalkLength = fElapsedTime * m_fWalkSpeed;
		float fDistance = (m_vPos - m_vNextPos).length();
		// ----
		while (fDistance<fWalkLength)
		{
			// ----
			if(m_Path.size() == 0)
			{
				m_vPos = m_vNextPos;
				setActionState(STAND);
				// ----
				setDir(m_uTargetDir);
				fWalkLength = 0;
				break;
			}
			else
			{
				moveStep();
				// ----
				fWalkLength -= fDistance;
				fDistance = (m_vPos - m_vNextPos).length();
			}
		}
		if (fDistance>fWalkLength)
		{
			m_vPos 		+= (m_vNextPos - m_vPos)*(fWalkLength/fDistance);
			m_vPos.y	= CWorld::getInstance().getHeight(m_vPos.x, m_vPos.z);
		}
		//m_AnimMgr.uFrame / (float)m_AnimMgr.uTotalFrames
		//playWalkSound();
	}
	else if(m_uActionState == STAND)
	{
		if(m_Path.size() > 0)
		{
			moveStep();
		}
	}
	else if(m_uActionState == HIT1)
	{
		if(m_AnimMgr.CurLoop > 0)
		{
			// ----
			sprintf_s(m_szTemp, "Data\\Sound\\eMeleeHit%d.wav", (rand() % 4) + 1);
			// ----
			// # Play Hit Sound.
			// ----
			GetAudio().playSound(m_szTemp);
			// # Set stand state.
			// ----
			setActionState(STAND);
		}
	}
	else if(m_uActionState==DIE)
	{
		if(m_AnimMgr.CurLoop > 0)
		{
			/*if (getID()==CPlayerMe::getInstance().getID())
			{
				setActionState(STAND);
			}*/
			// ----
			setActionState(DEATH);
			return;
		}
	}
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

unsigned char GetDir(const Vec3D & vSrcDir, const Vec3D & vDestDir)
{
	Vec3D vDir	= vDestDir-vSrcDir;
	vDir.y		= 0.0f;
	// ----
	vDir.normalize();
	// ----
	float fDot	= vDir.dot(Vec3D(0.0f ,0.0f, 1.0f));
	// ----
	fDot		+=1.0f;
	// ----
	if(vDir.x < 0)
	{
		fDot =4.0f - fDot;
	}
	// ----
	return (unsigned char)(fDot * 2.0f + 0.5f);
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------

void CRole::setCharacterData(const CHARACTER_DATA& data)
{
	memcpy(&m_CharacterData,&data,sizeof(CHARACTER_DATA));
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------