#pragma once
#include "Memory.h"
#include "BumiiData.h"

namespace DataTypes
{
	class BumiiAccess
	{
	public:
		void Initialize(BumiiData bumii) {
			mappings = {
			{ 1033785125, {
					{ 598924800, &bumii.ffsd.no_use_ffsd},
					{ -1931585751, &bumii.ffsd.type}
				}
			},
			{ -609743950, {
					{ -630211665, &bumii.body.race},
					{ -1931585751, &bumii.body.type},
					{ -1768939692, &bumii.body.number},
					{ 130897217, &bumii.body.weight},
					{ -179444465, &bumii.body.height}
				}
			},
			{ -242586236, {
					{ 476640891, &bumii.personal.sex_age},
					{ 119909474, &bumii.personal.fav_color},
					{ -1522013957, &bumii.personal.sub_color_1},
					{ 1011792193, &bumii.personal.sub_color_2},
					{ -2027282111, &bumii.personal.head_fav_color},
					{ -2070008958, &bumii.personal.shoulder_fav_color},
					{ -1998221777, &bumii.personal.shoulder_sub_color_1}
				}
			},
			{ -437489583, {
					{ 204834153, &bumii.common.backpack},
					{ -1844728759, &bumii.common.hat},
					{ 690487320, &bumii.common.no_hat_always},
					{ 585552429, &bumii.common.body_correct},
					{ -278450728, &bumii.common.is_mid_age},
					{ 1246217209, &bumii.common.rot_cravicle},
					{ -417702514, &bumii.common.rot_arm},
					{ 177513301, &bumii.common.rot_leg},
					{ 1595755806, &bumii.common.rot_crotch}
				}
			},
			{ -583991336, {
					{ 143010205, &bumii.shape.jaw},
					{ 226532768, &bumii.shape.wrinkle},
					{ 449607278, &bumii.shape.make},
					{ 1861753562, &bumii.shape.trans_v},
					{ -330947196, &bumii.shape.scale},
					{ 1766047850, &bumii.shape.skin_color}
				}
			},
			{ -1661509351, {
					{ -1931585751, &bumii.hair.type},
					{ 1716930793, &bumii.hair.color},
					{ -1694571435, &bumii.hair.flip}
				}
			},
			{ 1920501681, {
					{ -1931585751, &bumii.eye.type},
					{ 1716930793, &bumii.eye.color},
					{ 1861753562, &bumii.eye.trans_v},
					{ -135181472, &bumii.eye.trans_u},
					{ 931746316, &bumii.eye.rotate},
					{ -330947196, &bumii.eye.scale},
					{ -259800235, &bumii.eye.aspect},
					{ 866878866, &bumii.eye.eyeball_trans_u},
					{ -1432169432, &bumii.eye.eyeball_trans_v},
					{ 103552164, &bumii.eye.eyeball_scale},
					{ 928495654, &bumii.eye.highlight_bright}
				}
			},
			{ 1170574555, {
					{ 829051577, &bumii.eye_ctrl.base_offset},
					{ -1526311843, &bumii.eye_ctrl.translim_out},
					{ -541841377, &bumii.eye_ctrl.translim_in},
					{ -1524838311, &bumii.eye_ctrl.translim_d},
					{ -810749781, &bumii.eye_ctrl.translim_u},
					{ -613296366, &bumii.eye_ctrl.neck_offset_ud}
				}
			},
			{ -750581687, {
					{ -1931585751, &bumii.eyebrow.type},
					{ 1716930793, &bumii.eyebrow.color},
					{ 1861753562, &bumii.eyebrow.trans_v},
					{ -135181472, &bumii.eyebrow.trans_u},
					{ 931746316, &bumii.eyebrow.rotate},
					{ -330947196, &bumii.eyebrow.scale},
					{ -259800235, &bumii.eyebrow.aspect}
				}
			},
					{ -2130940717, {
					{ -1931585751, &bumii.nose.type},
					{ 1861753562, &bumii.nose.trans_v},
					{ -330947196, &bumii.nose.scale}
				}
			},
			{ -1631232873, {
					{ -1931585751, &bumii.mouth.type},
					{ 1716930793, &bumii.mouth.color},
					{ 1861753562, &bumii.mouth.trans_v},
					{ -330947196, &bumii.mouth.scale},
					{ -259800235, &bumii.mouth.aspect}
				}
			},
			{ 938199843, {
					{ 1217346952, &bumii.beard.mustache},
					{ -330947196, &bumii.beard.scale},
					{ -1931585751, &bumii.beard.type},
					{ 1716930793, &bumii.beard.color}
				}
			},
			{ 416005983, {
					{ -1931585751, &bumii.glass.type},
					{ 1716930793, &bumii.glass.color}
				}
			},
			{ 759392697, {
					{ 2138030896, &bumii.korog.mask},
					{ 1766047850, &bumii.korog.skin_color},
					{ -1593879254, &bumii.korog.left_plant},
					{ 1407817330, &bumii.korog.right_plant}
				}
			},
			{ -1854981092, {
					{ 1766047850, &bumii.goron.skin_color}
				}
			},
			{ -1166969158, {
					{ -1661509351, &bumii.gerudo.hair},
					{ -2091006383, &bumii.gerudo.hair_color},
					{ 416005983, &bumii.gerudo.glass},
					{ 811080285, &bumii.gerudo.glass_color},
					{ 1766047850, &bumii.gerudo.skin_color},
					{ 574490739, &bumii.gerudo.lip_color}
				}
			},
			{ 824870815, {
					{ -977286111, &bumii.rito.body_color},
					{ -2091006383, &bumii.rito.hair_color}
				}
			},
			{ 1106336067, {
					{ -977286111, &bumii.zora.body_color}
				}
			}
			};
		};

		void WriteMiiData (int specificHeader = 0, int specificParameter = 0)
		{
			if (!Initialized && bumiiFileStartAddr <= 0)
			{
				Logging::LoggerService::LogWarning("Bumii service not started... Cancelling bumii setup.", __FUNCTION__);
				return;
			}

			bool found = false;
			int iterator = 0;

			while (!found && iterator < 1000)
			{
				int val = readIntFromMemory(bumiiFileStartAddr + (iterator * 4));

				if (mappings.count(val))
				{
					found = true;
					break;
				}

				iterator += 1;
			}

			if (iterator > 1000)
			{
				Logging::LoggerService::LogWarning("Could not find bumii headers.", __FUNCTION__);
				return;
			}

			uint64_t StartOfHeaders = bumiiFileStartAddr + (iterator * 4);

			Logging::LoggerService::LogInformation("Analyzing bumii headers...", __FUNCTION__);

			for (int i = 0; i < 18; i++)
			{
				uint64_t headerAddr = StartOfHeaders + (i * 8);

				int header = (uint64_t)readIntFromMemory(headerAddr);
				int childNum = ShortToIntFromMemory(headerAddr + 4);
				int childStart = ShortToIntFromMemory(headerAddr + 6) * 4;

				if (specificHeader != 0 && header != specificHeader) 
				{
					continue;
				}

				for (int j = 0; j < childNum; j++)
				{
					uint64_t childHeaderAddr = headerAddr + childStart + (j * 8);
					int param = (uint64_t)readIntFromMemory(childHeaderAddr);
					int paramOffset = ParamOffsetToIntFromMemory(childHeaderAddr + 5) * 4;
					
					if (mappings[header].count(param))
					{
						if (specificParameter != 0 && param != specificParameter)
						{
							continue;
						}

						// TODO: Review, we are writing into an invalid address

						if (param != 829051577) // For now, we will ignore Vec3
						{
							int val;

							memcpy(&val, mappings[header][param], 4);
							SwapEndianness<int>(val);
							memcpy((DWORD*)(childHeaderAddr + paramOffset), &val, 4);

							std::stringstream stream;
							stream << "Param: " << param << " Param offset: " << std::hex << paramOffset << " Start of child header: " << std::hex << childHeaderAddr;
							stream << " Value: " << val;
							Logging::LoggerService::LogDebug(stream.str(), __FUNCTION__);
						}

						if (param == specificParameter)
						{
							break;
						}
					}
				}
			}
		}

		int readIntFromMemory(uint64_t addr)
		{
			int val;

			// Copy value in memory to val
			memcpy(&val, (DWORD*)(addr), 4);
			// Swap from little to big endian
			SwapEndianness<int>(val);

			return val;
		}

		int ShortToIntFromMemory(uint64_t addr)
		{
			short val;
			memcpy(&val, (DWORD*)(addr), 2);
			
			SwapEndianness<short>(val);

			return val;
		}

		int ParamOffsetToIntFromMemory(uint64_t addr) 
		{
			int val;
			memcpy(&val, (DWORD*)(addr + 2), 1);
			memcpy(&val + 1, (DWORD*)(addr + 1), 1);
			memcpy(&val + 2, (DWORD*)(addr), 1);

			return val;
		}

		void setAddress(uint64_t baseAddr)
		{
			bumiiFileStartAddr = Memory::ReadPointers(baseAddr, { 0x39C, 0xC4 }, true);
		}

	private:
		bool Initialized = false;
		uint64_t bumiiFileStartAddr = 0;
		std::map<int, std::map<int, void*>> mappings;

		template <typename T>
		void SwapEndianness(T& val)
		{
			union U {
				T val;
				std::array<std::uint8_t, sizeof(T)> raw;
			} src, dst;

			memcpy(&src.val, &val, sizeof(val));
			std::reverse_copy(src.raw.begin(), src.raw.end(), dst.raw.begin());
			memcpy(&val, &dst.val, sizeof(val));
		}
	};
}