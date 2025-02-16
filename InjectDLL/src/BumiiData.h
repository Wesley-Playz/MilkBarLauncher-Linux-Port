#pragma once
#include "Vec3f.h"
#include <string>

namespace DataTypes
{
	class BumiiData
	{
    public:
        class FFSD
        {
        public:
            int no_use_ffsd;
            int type;
        };

        class BODY
        {
        public:
            int race;
            int type;
            int number;
            int weight;
            int height;
        };

        class PERSONAL
        {
        public:
            int sex_age;
            int fav_color;
            int sub_color_1;
            int sub_color_2;
            int head_fav_color;
            int shoulder_fav_color;
            int shoulder_sub_color_1;
        };

        class COMMON
        {
        public:
            int backpack;
            int hat;
            int no_hat_always;
            int body_correct;
            int is_mid_age;
            float rot_cravicle;
            float rot_arm;
            float rot_leg;
            float rot_crotch;
        };

        class SHAPE
        {
        public:
            int jaw;
            int wrinkle;
            int make;
            float trans_v;
            float scale;
            int skin_color;
        };

        class HAIR
        {
        public:
            int type;
            int color;
            int flip;
        };

        class EYE
        {
        public:
            int type;
            int color;
            float trans_v;
            float trans_u;
            float rotate;
            float scale;
            float aspect;
            float eyeball_trans_u;
            float eyeball_trans_v;
            float eyeball_scale;
            int highlight_bright;
        };

        class EYE_CTRL
        {
        public:
            Vec3f base_offset;
            float translim_out;
            float translim_in;
            float translim_d;
            float translim_u;
            float neck_offset_ud;
        };

        class EYEBROW
        {
        public:
            int type;
            int color;
            float trans_v;
            float trans_u;
            float rotate;
            float scale;
            float aspect;
        };

        class NOSE
        {
        public:
            int type;
            float trans_v;
            float scale;
        };

        class MOUTH
        {
        public:
            int type;
            int color;
            float trans_v;
            float scale;
            float aspect;
        };

        class BEARD
        {
        public:
            int mustache;
            float scale;
            int type;
            int color;
        };

        class GLASS
        {
        public:
            int type;
            int color;
        };

        class KOROG
        {
        public:
            int mask;
            int skin_color;
            int left_plant;
            int right_plant;
        };

        class GORON
        {
        public:
            int skin_color;
        };

        class GERUDO
        {
        public:
            int hair;
            int hair_color;
            int glass;
            int glass_color;
            int skin_color;
            int lip_color;
        };

        class RITO
        {
        public:
            int body_color;
            int hair_color;
        };

        class ZORA
        {
        public:
            int body_color;
        };

        FFSD ffsd;
        BODY body;
        PERSONAL personal;
        COMMON common;
        SHAPE shape;
        HAIR hair;
        EYE eye;
        EYE_CTRL eye_ctrl;
        EYEBROW eyebrow;
        NOSE nose;
        MOUTH mouth;
        BEARD beard;
        GLASS glass;
        KOROG korog;
        GORON goron;
        GERUDO gerudo;
        RITO rito;
        ZORA zora;
	};
}