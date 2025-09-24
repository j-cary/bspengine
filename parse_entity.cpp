#include "entity.h"
#include "md2.h" //md2list
#include "pmove.h" //droptofloor



//entlist stuff
//entities added after worldspawn will be done manually

#define KT_INT	0
#define KT_VEC	1
#define KT_FLT	2
#define KT_STR	3
#define KT_LGT	4 //four wide vector



void IntTranslate(baseent_c* ent, char* val, int ofs)
{
	int* var = (int*)(ent)+ofs / sizeof(int);
	//printf("ent %p is calling inttranslate with val %s and ofs %i\n", ent, val, ofs);

	*var = atoi(val);
}

void VecTranslate(baseent_c* ent, char* val, int ofs)
{
	float* var = (float*)(ent)+ofs / sizeof(float);
	char read[16] = {};
	int r = 0;

	//printf("ent %p is calling vectranslate with val %s and ofs %i\n", ent, val, ofs);


	for (; *val; val++)
	{
		if (*val == ' ')
		{
			*var = (float)atof(read);
			memset(read, 0, 16);
			r = 0;
			var++;
		}
		else //presumably this is always a number
		{

			read[r] = *val;
			r++;
		}
	}

	//swap y & z
	*var = *(var - 1);
	*(var - 1) = (float)atof(read); //this isn't caught by the space check
}

void FltTranslate(baseent_c* ent, char* val, int ofs)
{
	float* var = (float*)(ent)+ofs / sizeof(float);
	//printf("ent %p is calling flttranslate with val %s and ofs %i\n", ent, val, ofs);

	//CHECKME!!!!
	*var = (float)atof(val);
}

void StrTranslate(baseent_c* ent, char* val, int ofs)
{
	char* var = (char*)(ent)+ofs;
	//printf("ent %p is calling strtranslate with val %s and ofs %i\n", ent, val, ofs);

	strcpy(var, val);
}

void LgtTranslate(baseent_c* ent, char* val, int ofs)
{
	float* var = (float*)(ent)+ofs / sizeof(float);
	char read[16] = {};
	int r = 0;

	//printf("ent %p is calling lgttranslate with val %s and ofs %i\n", ent, val, ofs);


	for (; *val; val++)
	{
		if (*val == ' ')
		{
			*var = (float)atof(read);
			memset(read, 0, 16);
			r = 0;
			var++;
		}
		else //presumably this is always a number
		{

			read[r] = *val;
			r++;
		}
	}

	*var = (float)atof(read); //this isn't caught by the space check
}

keytranslate_t spawnkeys[] =
{
	"classname",	&StrTranslate,	offsetof(baseent_c, classname),
	"name",			&StrTranslate,	offsetof(baseent_c, name),
	"origin",		&VecTranslate,	offsetof(baseent_c, origin),
	"angles",		&VecTranslate,	offsetof(baseent_c, angles),
	"spawnflags",	&IntTranslate,	offsetof(baseent_c, flags),

	"_light",		&LgtTranslate,	offsetof(baseent_c, light),
	"model",		&StrTranslate,	offsetof(baseent_c, modelname),
	"noise",		&StrTranslate,	offsetof(baseent_c, noise),
	"frame",		&IntTranslate,	offsetof(baseent_c, models[0].frame),
	NULL,			NULL,			0,
};

#define HSK(_class, key, transfunc)			#key, &transfunc, offsetof(_class, key) //Hammer Spawn Key convenience macro
#define HSK2(_class, hkey, key, transfunc)	hkey, &transfunc, offsetof(_class, key)

keytranslate_t basespawnkeys[] =
{
	"classname", &StrTranslate, offsetof(baseent_c, classname),
	"name", &StrTranslate, offsetof(baseent_c, name),
	"origin", &VecTranslate, offsetof(baseent_c, origin),
	HSK(baseent_c, angles, VecTranslate),
	HSK2(baseent_c, "spawnflags", flags, IntTranslate),
	NULL, NULL, 0 //either name or func can be used as a terminator
};



void baseent_c::SerializeHammerSpawnKeys(keytranslate_s* spawnkeys, std::vector<hammerkv_t*>& keyvals)
{
	for (int i = (int)keyvals.size() - 1; i >= 0; i--)
	{
		hammerkv_t* at = keyvals[i];
		for (keytranslate_t* kt = spawnkeys; kt->translatefunc; kt++)
		{
			if (!strcmp(kt->name, at->key))
			{
				kt->translatefunc(this, at->val, kt->ofs);
				keyvals.erase(keyvals.begin() + i); //handled this pair, delete the entry
				break;
			}
		}
	}
}

void baseent_c::HammerSpawn(std::vector<hammerkv_t*>& keyvals)
{
	SerializeHammerSpawnKeys(basespawnkeys, keyvals);
}

//temp
extern bsp_t bsp;
void ent::solid_c::HammerSpawn(std::vector<hammerkv_t*>& keyvals)
{
	baseent_c::HammerSpawn(keyvals);

	//check this out. just assuming that the number following '*' is the index into models
	int i = atoi(&modelname[1]); //skip '*'
	bmodel = &bsp.models[i];
}

keytranslate_t modelspawnkeys[] =
{
	HSK2(ent::model_c, "model", modelname, StrTranslate),
	HSK2(ent::model_c, "frame", models[0].frame, IntTranslate),
	NULL, NULL, 0
};

#undef HSK
#undef HSK2

extern md2list_c md2list; //tmp
void ent::model_c::HammerSpawn(std::vector <hammerkv_t*>& keyvals)
{
	baseent_c::HammerSpawn(keyvals);
	SerializeHammerSpawnKeys(modelspawnkeys, keyvals);

	if (!*modelname)
		printf("'model' with no modelname!\n");
	else
		models[0].mid = md2list.Alloc(modelname, this, &models[0]);
}

//entities are separated by curly braces. Their attributes are contained within lines. These keyvalue pairs are contained within quotes and separated by spaces.
//len includes the final newline as well

void ParseHammerEntity(char* start, int len)
{
	//char	key[16];
	//char	val[128];
	char* k;
	char* v;
	baseent_c* e = NULL;

	hammerkv_t	kv[16];
	int			count = 0;
	std::vector<hammerkv_t*> list;

	//e = AllocEnt();

	for (; len > 0; len--, start++)
	{
		if (*start == '"')
		{
			
			v = kv[count].val; //v = val;
			k = kv[count].key; //k = key;

			//skip the first quote
			for (start++, len--; *start != '"'; len--, start++)
			{//get the key
				*k = *start;
				k++;
			}

			//skip the " " in between 
			for (start += 3, len -= 3; *start != '"'; len--, start++)
			{//get the val
				*v = *start;
				v++;
			}

			*v = *k = '\0';
			//VarKV(e, key, val);
			list.push_back(&kv[count]);
			count++;

			//skip the last quote
			start++;
			len--;

			//printf("%s - %s\n", key, val);
		}
	}

	//
	for (int i = 0; i < count; i++)
	{
		if (!strcmp("classname", kv[i].key))
		{
			e = AllocEnt(kv[i].val);
			break;
		}
	}

	if (!e)
		SYS_Exit("Entity without classname!\n");

	//ents call their base class' version of this function before running their own.
	//elements from the lists are removed as they are handled
	e->HammerSpawn(list);
	//e->AddHammerEntity();
}

void LoadHammerEntities(char* str, int len)
{//https://www.gamers.org/dEngine/quake/spec/quake-spec34/qkspec_2.htm#2.2.3
	char* start = NULL;
	int ent_len = 0;

	for (; len > 0; len--, str++, ent_len++)
	{
		if (*str == '{')
		{
			ent_len = 0;
			start = str + 1;

			if (*start == '{') //this might just be a thing in .MAP files...
				SYS_Exit("LoadHammerEntities is not prepared to handle brush info\n");
		}

		if (*str == '}')
		{
			ParseHammerEntity(start, ent_len - 2); //cut off the "\n}" pair
		}
	}
}