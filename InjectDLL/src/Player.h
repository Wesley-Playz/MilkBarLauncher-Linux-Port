#pragma once

#include "Actor.h"
#include "EquipmentAccess.h"
#include "BumiiAccess.h"
#include "Extrapolation.h"
#include "LocalInstance.h"
#include "Vec3f_Operations.h"

namespace MemoryAccess
{
	class Player : public Actor
	{
	public:
		int PlayerNumber;
		bool isFar = true;
		bool connected = false;
		std::string Name = "";
		ModelData Model = ModelData();
		int ping = 0;
		DWORD LastUpdated = 0;
		DWORD LastAttack = 0;
		int LastAnimation = 0;
		int ChargeAttackRetries = 0;

		Vec3f Speed = Vec3f(0, 0, 0);
		Vec3f LastUpdatedPosition = Vec3f(0, 0, 0);
		Vec3f LastServerPosition = Vec3f(0, 0, 0);
		float AtkUp = 1;
		int Health = 0;

		CharacterLocation* Location = new CharacterLocation();
		EquipmentAccess* Equipment = new EquipmentAccess();
		BumiiAccess* Bumii = new BumiiAccess();
		LittleEndian<bool>* Exists;
		LittleEndian<bool>* DispName;
		BigEndian<int>* Status;
		uint64_t NameAddr;
		uint64_t AnimAddr;
		uint64_t HoldAddr;
		Vec3fBE* MapPin;
		uint64_t AttackAddr;
		BombAccess* Bomb = new BombAccess();
		BombAccess* Bomb2 = new BombAccess();
		BombAccess* BombCube = new BombAccess();
		BombAccess* BombCube2 = new BombAccess();

		bool HideFromMap = false;

		MemoryAccess::LocalInstance* GameInstance;

		std::thread pThread;
		bool RunThread = false;

		enum ActionEnum
		{
			ActSkip,
			ActCreate,
			ActDelete
		};

		Player(byte playerNumber, MemoryAccess::LocalInstance* instance, MemoryAccess::LocalInstance::FlagAddresses actorFlags)
		{
			GameInstance = instance;
			PlayerNumber = playerNumber;

			Exists = new LittleEndian<bool>(actorFlags.ExistsFlag, __FUNCTION__);
			DispName = new LittleEndian<bool>(actorFlags.DispNameFlag, __FUNCTION__);
			Status = new BigEndian<int>(actorFlags.StatusFlag, __FUNCTION__);
			Status->set(0, __FUNCTION__);
			NameAddr = actorFlags.NameFlag;
			AnimAddr = actorFlags.AnimationFlag;
			HoldAddr = actorFlags.HoldFlag;
			MapPin = new Vec3fBE(actorFlags.MapPin, __FUNCTION__);
			AttackAddr = actorFlags.AttackAnimation;
		}

		void setAddress(uint64_t addr)
		{
			Mutex.lock();
			Actor::setAddress(addr);
			Mutex.unlock();
		}

		void set(DTO::CloseCharacterDTO* PlayerData, bool DispNames, bool isPaused)
		{
			if (!connected)
				Connect();

			isFar = false;
			Location->Map = PlayerData->Location.Map;
			Location->Section = PlayerData->Location.Section;

			if (!isPaused)
				UpdateName(DispNames && !HideFromMap);

			UpdateMapPin(PlayerData->Position, !HideFromMap);

			LastServerPosition = PlayerData->Position;

			if (this->baseAddr == 0 || !PlayerData->Updated)
				return;

			const std::string DEFAULT_ANIM = "Jugador" + std::to_string(PlayerNumber) + "_animationthing";
			const std::string DEFAULT_ATTACK = "Jugador32_AttackAnimation";

			/* Attack detection
			Animations stored in AttackAnimations are the animations we have identified as attack such as Sword_Attack_S1
			*/
			std::vector<int> AttackAnimations = { 928165310, 1294448978, 583341237, 897962260, -1903343953, -2023673433, 847655708, -1589496633, 2146177340, -1303343253, -123813806, 1543951008, 1128528738, -1248213324, 1874126347, 806287798, -742419528, -1630162536, 1600181768, 1039952503, -1888311262, -1480116640, -1927143427, -533414859, -36670094, -2108440740, -1884523108, -1615366629, -1246603057, -1470721656, -1996120276, -1797171605, -2076564023, -112340596, 728159080, -458680117, -1713443698, 1542065194, 647886959, -781586933, 997798184, 1175373165, -393452632, 554609132, -1130259111, 1021993131, -2109979814, -30085678, -18972657, 1185849402, -482719416, -1803236064, -1884724833, -1840404264, -1342573430, -1543868860, -1306706483, -1743252711, -2047026594, -426229114, -75723839, -2108575578, -14269213, -1827922336, -490566236, -1615496735, 1568774469, 553020672, 1761813251, 1024631879, 1080381442, -765641975, -207435949, -563725248, -297398764, -1192420357, -991578765, 745729712, 610861347, 831924215, -1370122879, 835249056, -176172835, 463333429, 259059134, -25556495, -1508973864, 74790034, -629870140, 1754272119, 1320358935, 789467239, 273082485, -1044574760, 780998171, 1752946335, -1169493048, 1414384225, 1199510679, 1767142188, 953715144, -1064130453, 964742086, -716436526, 1310776481, -447236380, -1742716255, -122454109, -646716027, -2145874292, 773977556, 710297995, 1201725498, -277486945, -813687283, 1234073523, 1553707875, -796449271, 1695977382, -1484777825, 1919785957, 971277116, 369948204, 1323545514, 24633967, 131976475, -1630233439, 747673358, -1029769694, 1559913345, 738022167, -1292631379, -973917637, -916295166, -996204138, -1025054081, -585303469, -576147724, -1259644422, -1747463897, -831621317, -432033607, 811876277, 1656633699, 791313214, 33565683, -1695879491, 554960384, 1531097105, 468014366, 353438393, 1046237356, -632640808, 131165289, -1693877687, -1553477000, -1206036550, 1657769172, -334607649, -1273182661, -820353236, 1974769815, -1432420688, -712724095, -2116836187, -866005140, 237519009, -1257045322, 22218317, -1492002104, -1798676531, 1578196338, 153657744, -1165622557, 140698192, 776070960 };

			bool IsAttack = false;

			if (std::find(AttackAnimations.begin(), AttackAnimations.end(), PlayerData->Animation) != AttackAnimations.end()) 
			{
				LastAttack = GetTickCount();
				IsAttack = true;
			}

			Teleport(PlayerData->Position, true);

			Rotation1->set(PlayerData->Rotation1, __FUNCTION__);
			Rotation2->set(PlayerData->Rotation2, __FUNCTION__);
			Rotation3->set(PlayerData->Rotation3, __FUNCTION__);
			Rotation4->set(PlayerData->Rotation4, __FUNCTION__);
			Rotation5->set(PlayerData->Rotation4, __FUNCTION__);

			if (isPaused)
				return;

			bool IsCharge = false;

			if (PlayerData->Animation == 806287798)
			{
				IsAttack = false;

				if (LastAnimation != 806287798)
				{
					ChargeAttackRetries = 0;
				}
				else
				{
					ChargeAttackRetries++;
					
					if (ChargeAttackRetries > 10)
					{
						IsAttack = true;
						IsCharge = true;
					}
				}
			}

			if (PlayerData->Animation == 835249056)
				IsAttack = false;

			if (this->Status->get(__FUNCTION__) != 1)
			{
				if (IsAttack && (IsCharge || PlayerData->Animation != LastAnimation))
				{
					this->Status->set(2, __FUNCTION__);
					Memory::write_string(AttackAddr, "Attack_" + std::to_string(UINT32(PlayerData->Animation)), DEFAULT_ATTACK.size(), __FUNCTION__);
				}
				else
				{
					this->Status->set(0, __FUNCTION__);
					Memory::write_string(AnimAddr, "Anim_" + std::to_string(UINT32(PlayerData->Animation)), DEFAULT_ANIM.size(), __FUNCTION__);
				}
			}
			
			LastAnimation = PlayerData->Animation;

			Memory::write_string(HoldAddr, PlayerData->IsEquipped ? "Hold" : "Equip", 6, __FUNCTION__);
			this->Equipment->Changed = Equipment->Compare(PlayerData->Equipment);
			AtkUp = PlayerData->AtkUp;
			Health = PlayerData->Health;

			ManageBomb(PlayerData->Bomb, 0);
			ManageBomb(PlayerData->Bomb2, 1);
			ManageBomb(PlayerData->BombCube, 2);
			ManageBomb(PlayerData->BombCube2, 3);
		}

		void set(DTO::FarCharacterDTO* PlayerData, bool DispNames, bool isPaused)
		{
			if (!connected)
				Connect();

			this->Health = 0;
			isFar = true;

			UpdateMapPin(PlayerData->Position, !HideFromMap);

			if (isPaused)
				return;

			UpdateName(DispNames);
			this->Status->set(1, __FUNCTION__);

		}

		void setName(std::string newName)
		{
			Memory::write_string(NameAddr, newName, 32, __FUNCTION__);
		}

		void Teleport(Vec3f newPosition, bool Save = false)
		{
			if (!Save)
			{
				Actor::Teleport(newPosition);
				return;
			}

			Vec3f OldPosition = this->Position->LastKnown;
			Vec3f PosOffsetPcnt = Vec3f();

			for (int i = 0; i < 3; i++)
			{
				PosOffsetPcnt[i] = (newPosition[i] - OldPosition[i]) / newPosition[i];
				if (PosOffsetPcnt[i] > 1)
				{
					PosOffsetPcnt[i] = 0;
				}
			}

			if (LastUpdated == 0)
			{
				Speed = Vec3f(0, 0, 0); // Set speed to 0 as we are just starting
			}
			else
			{
				Vec3f newSpeed = Helper::Extrapolation::CalcSpeed(LastUpdatedPosition, newPosition, float(GetTickCount() - LastUpdated) / 1000); // Returns Vec3f in distanceUnits/seconds
				if (Helper::Vec3f_Operations::Equals(newSpeed, Vec3f(0, 0, 0)))
				{
					Speed = newSpeed;
					Actor::Teleport(newPosition);

					LastUpdated = GetTickCount();
					LastUpdatedPosition = newPosition;
					return;
				}

				Speed = Vec3f((Speed.x() + newSpeed.x())/ 2, (Speed.y() + newSpeed.y()) / 2, (Speed.z() + newSpeed.z()) / 2);
			}

			Vec3f SpeedFromLastKnown = Helper::Extrapolation::CalcSpeed(OldPosition, newPosition, 1); // Value of 1 because we are not looking for the actual speed

			for (int i = 0; i < 3; i++)
			{
				Speed[i] += Speed[i] * PosOffsetPcnt[i]; //TODO: Test more deeply

				if (Helper::Vec3f_Operations::GetSigns(SpeedFromLastKnown[i]) != Helper::Vec3f_Operations::GetSigns(Speed[i]) && abs(OldPosition[i] - newPosition[i]) < 0.2)
				{
					Speed[i] = 0;
				}

				if (abs(OldPosition[i] - newPosition[i]) > 3)
				{
					Actor::Teleport(newPosition);
				}
			}

			LastUpdated = GetTickCount();
			LastUpdatedPosition = newPosition;
		}

		void Connect()
		{
			//this->Delete->set(false, __FUNCTION__);
			this->Status->set(0, __FUNCTION__);
			connected = true;
			if (PlayerNumber != 32)
			{
				Logging::LoggerService::LogInformation("Player " + Name + " joined.", __FUNCTION__);
				Memory::MessagerService::AddMessage("Player " + Name + " joined.");
			}
			this->RunThread = true;
			pThread = std::thread(&Player::PThread, this);
		}

		void Disconnect()
		{
			connected = false;
			if (PlayerNumber != 32)
			{
				Logging::LoggerService::LogInformation("Player " + Name + " left.", __FUNCTION__);
				Memory::MessagerService::AddMessage("Player " + Name + " left.");
			}
			UpdateMapPin(Vec3f(0, 0, 0), false);
			Mutex.lock();
			this->baseAddr = 0;
			//this->Delete->set(true, __FUNCTION__);
			this->Status->set(1, __FUNCTION__);
			Mutex.unlock();
			this->RunThread = false;
			pThread.join();
		}

		void UpdateName(bool DispNames)
		{
			std::string ActualName = this->Name;

			if (Name.size() > 19)
				ActualName = Name.substr(0, 19);

			if(this->Health > 0)
				ActualName += " HP: " + std::to_string(this->Health);

			Memory::write_string(NameAddr, ActualName, 32, __FUNCTION__);
			this->DispName->set(DispNames, __FUNCTION__);
		}

	private:
		std::shared_mutex Mutex;

		void PThread();

		void ManageBomb(Vec3f BombData, int bomb)
		{
			BombAccess* Bomb = new BombAccess();
			std::string BombName;

			if (bomb == 0)
			{
				Bomb = this->Bomb;
				BombName = "CustomRemoteBomb";
			}
			else if (bomb == 1)
			{
				Bomb = this->Bomb2;
				BombName = "CustomRemoteBomb2";
			}
			else if (bomb == 2)
			{
				Bomb = this->BombCube;
				BombName = "CustomRemoteBombCube";
			}
			else if (bomb == 3)
			{
				Bomb = this->BombCube2;
				BombName = "CustomRemoteBombCube2";
			}

			BombStatus newStatus = Deallocated;
			BombStatus playerStatus = Bomb->getStatus();

			if (BombData.x() == 0 && BombData.y() == 0 && BombData.z() == 0)
				newStatus = Exploded;
			else if (BombData.x() == -1 && BombData.y() == -1 && BombData.z() == -1)
				newStatus = Cancelled;
			else
				newStatus = Normal;

			if (playerStatus == Normal)
			{
				if (newStatus != Normal)
				{
					Bomb->changeState(newStatus);
					return;
				}

				Bomb->set(BombData, __FUNCTION__);
			}
			else if (playerStatus == Deallocated)
			{
				if (newStatus == Normal && GameInstance->RequestCreate(BombName, BombData))
				{
					Bomb->changeState(Processing);
				}
			}
		}

		void UpdateMapPin(Vec3f newPosition, bool show)
		{
			if (show)
				MapPin->set(newPosition, __FUNCTION__);
			else
				MapPin->set(Vec3f(12340.5, 2345.5, 0), __FUNCTION__);
		}
	};
}