#include <cstdint>
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <vector>
#include <vitasdk.h>

#include "game.h"
#include "sfo.h"
#include "fs.h"
#include "windows.h"
#include "textures.h"
#include "db.h"

#define GAME_LIST_FILE "ux0:data/SMLA00001/games_list.txt"
#define FAVORITES_FILE "ux0:data/SMLA00001/favorites.txt"
#define NUM_CACHED_PAGES 5

GameCategory game_categories[TOTAL_CATEGORY];
GameCategory *current_category;

bool game_scan_complete = false;
int games_to_scan = 1;
Game game_scan_inprogress;
std::string vita = "vita";
std::string psp = "psp";
std::string ps1 = "ps1";
std::string psmini = "psmini";
std::string psmobile = "psmobile";
std::string nes = "nes";
std::string snes = "snes";
std::string gb = "gb";
std::string gba = "gba";
std::string n64 = "n64";
std::string ports = "ports";
std::string original = "original";
std::string utilities = "utilities";
std::string emulator = "emulator";
std::string homebrew = "homebrew";

bool use_game_db = true;

namespace GAME {

    void Init() {
    }

    bool GetGameDetails(const char *game_id, Game *game)
    {
        char sfo_file[256];
        sprintf(sfo_file, "ux0:app/%s/sce_sys/param.sfo", game_id);
        if (FS::FileExists(sfo_file)) {
            const auto sfo = FS::Load(sfo_file);

            std::string title = std::string(SFO::GetString(sfo.data(), sfo.size(), "TITLE"));
            std::replace( title.begin(), title.end(), '\n', ' ');

            sprintf(game->id, "%s", game_id);
            sprintf(game->title, "%s", title.c_str());
            sprintf(game->icon_path, "ur0:appmeta/%s/icon0.png", game_id);
            sprintf(game->category, "%s", GetGameCategory(game_id));
            game->tex = no_icon;
            return true;
        }
        return false;
    }

    void Scan()
    {
        current_category = &game_categories[VITA_GAMES];

        if (use_game_db)
        {
            games_to_scan = DB::GetVitaDbGamesCount();
            DB::GetVitaDbGames(current_category);
        }
        else if (!FS::FileExists(GAME_LIST_FILE))
        {
            FS::MkDirs("ux0:data/SMLA00001");
            void* fd = FS::Create(GAME_LIST_FILE);

            std::vector<std::string> dirs = FS::ListDir("ux0:app/");
            games_to_scan = dirs.size();

            for(std::size_t i = 0; i < dirs.size(); ++i) {
                Game game;
                if (GetGameDetails(dirs[i].c_str(), &game))
                {
                    char line[512];
                    sprintf(line, "%s||%s||%s||%s\n", game.id, game.title, game.category, game.icon_path);
                    FS::Write(fd, line, strlen(line));
                    game_scan_inprogress = game;
                    current_category->games.push_back(game);
                }
            }
            FS::Close(fd);

        }
        else {
            LoadGamesCache();
        }

        LoadFavorites();

        bool favorites_updated = false;
        for (std::vector<Game>::iterator it=game_categories[FAVORITES].games.begin(); 
             it!=game_categories[FAVORITES].games.end(); )
        {
            Game* game = FindGame(current_category, it->id);
            if (game != nullptr)
            {
                game->favorite = true;
                ++it;
            }
            else
            {
                it = game_categories[FAVORITES].games.erase(it);
                favorites_updated = true;
            }
        }
        if (favorites_updated)
            SaveFavorites();
        
        
        for (std::vector<Game>::iterator it=current_category->games.begin();
            it!=current_category->games.end(); )
        {
            if (strcmp(it->category, psp.c_str()) == 0)
            {
                game_categories[PSP_GAMES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, ps1.c_str()) == 0)
            {
                game_categories[PS1_GAMES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, psmini.c_str()) == 0)
            {
                game_categories[PS_MIMI_GAMES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, psmobile.c_str()) == 0)
            {
                game_categories[PS_MOBILE_GAMES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, nes.c_str()) == 0)
            {
                game_categories[NES_GAMES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, snes.c_str()) == 0)
            {
                game_categories[SNES_GAMES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, gb.c_str()) == 0)
            {
                game_categories[GB_GAMES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, gba.c_str()) == 0)
            {
                game_categories[GBA_GAMES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, n64.c_str()) == 0)
            {
                game_categories[N64_GAMES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, ports.c_str()) == 0)
            {
                game_categories[PORT_GAMES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, original.c_str()) == 0)
            {
                game_categories[ORIGINAL_GAMES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, utilities.c_str()) == 0)
            {
                game_categories[UTILITIES].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, emulator.c_str()) == 0)
            {
                game_categories[EMULATORS].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else if (strcmp(it->category, homebrew.c_str()) == 0)
            {
                game_categories[HOMEBREWS].games.push_back(*it);
                it = current_category->games.erase(it);
                games_to_scan--;
            }
            else
            {
                ++it;
            }
        }
        
        for (int i=0; i < TOTAL_CATEGORY; i++)
        {
            SortGames(&game_categories[i]);
            game_categories[i].page_num = 1;
            SetMaxPage(&game_categories[i]);
        }
    }

    void SetMaxPage(GameCategory *category)
    {
        category->max_page = (category->games.size() + 18 - 1) / 18;
        if (category->max_page == 0)
        {
            category->max_page = 1;
        }
    }

    bool Launch(const char *title_id) {
        char game_path[96];
        sprintf(game_path, "ux0:app/%s", title_id);
       	char uri[32];
        sprintf(uri, "psgm:play?titleid=%s", title_id);
        sceAppMgrLaunchAppByUri(0xFFFFF, uri);
        sceKernelExitProcess(0);
        return true;
    };

    std::string nextToken(std::vector<char> &buffer, int &nextTokenPos)
    {
        std::string token = "";
        if (nextTokenPos >= buffer.size())
        {
            return NULL;
        }

        for (int i = nextTokenPos; i < buffer.size(); i++)
        {
            if (buffer[i] == '|')
            {
                if ((i+1 < buffer.size()) && (buffer[i+1] == '|'))
                {
                    nextTokenPos = i+2;
                    return token;
                }
                else {
                    token += buffer[i];
                }
            }
            else if (buffer[i] == '\n') {
                nextTokenPos = i+1;
                return token;
            }
            else {
                token += buffer[i];
            }
        }

        return token;
    }

    void LoadGamesCache() {
        std::vector<char> game_buffer = FS::Load(GAME_LIST_FILE);
        int position = 0;
        while (position < game_buffer.size())
        {
            Game game;
            sprintf(game.id, "%s", nextToken(game_buffer, position).c_str());
            sprintf(game.title, "%s", nextToken(game_buffer, position).c_str());
            sprintf(game.category, "%s", nextToken(game_buffer, position).c_str());
            sprintf(game.icon_path, "%s",  nextToken(game_buffer, position).c_str());
            game.tex = no_icon;
            current_category->games.push_back(game);
            games_to_scan = current_category->games.size();
        }
    };

    void SaveGamesCache()
    {
        void* fd = FS::Create(GAME_LIST_FILE);
        for (int j=1; j < TOTAL_CATEGORY; j++)
        {
            for (int i=0; i < game_categories[j].games.size(); i++)
            {
                Game* game = &game_categories[j].games[i];
                char line[512];
                sprintf(line, "%s||%s||%s||%s\n", game->id, game->title, game->category, game->icon_path);
                FS::Write(fd, line, strlen(line));
            }
        }
        FS::Close(fd);
    }

    void LoadGameImages(int category, int prev_page, int page) {
        int high = 0;
        int low = 0;

        if (game_categories[category].max_page > NUM_CACHED_PAGES + 5)
        {
            int del_page = 0;

            if ((page > prev_page) or (prev_page == game_categories[category].max_page && page == 1))
            {
                del_page = DecrementPage(page, NUM_CACHED_PAGES);
            } else if ((page < prev_page) or (prev_page == 1 && page == game_categories[category].max_page))
            {
                del_page = IncrementPage(page, NUM_CACHED_PAGES);
            }

            int high = del_page * 18;
            int low = high - 18;
            if (del_page > 0)
            {
                for (int i=low; (i<high && i < game_categories[category].games.size()); i++)
                {
                    Game *game = &game_categories[category].games[i];
                    if (game->tex.id != no_icon.id)
                    {
                        Textures::Free(&game->tex);
                        game->tex = no_icon;
                    }
                }
            }
        }

        high = page * 18;
        low = high - 18;
        for(std::size_t i = low; (i < high && i < game_categories[category].games.size()); i++) {
            Game *game = &game_categories[category].games[i];
            if (page == current_category->page_num && category == current_category->id)
            {
                if (game->tex.id == no_icon.id)
                {
                    LoadGameImage(game);
                }
            }
            else
            {
                // No need to continue if page isn't in view
                return;
            }
            
        }
    }

    int IncrementPage(int page, int num_of_pages)
    {
        int new_page = page + num_of_pages;
        if (new_page > current_category->max_page)
        {
            new_page = new_page % current_category->max_page;
        }
        return new_page;
    }

    int DecrementPage(int page, int num_of_pages)
    {
        int new_page = page - num_of_pages;
        if (new_page > 0)
        {
            return new_page;
        }
        return new_page + current_category->max_page;
    }

    void LoadGameImage(Game *game) {
        Tex tex;
        tex = no_icon;

        char custom_icon_path[64];
        sprintf(custom_icon_path, "ur0:data/SMLA00001/icons/%s.png", game->id);
        if (FS::FileExists(custom_icon_path))
        {
            if (Textures::LoadImageFile(custom_icon_path, &tex))
            {
                game->tex = tex;
            }
        }

        if (tex.id == no_icon.id && FS::FileExists(game->icon_path))
        {
            if (Textures::LoadImageFile(game->icon_path, &tex))
            {
                game->tex = tex;
            }
        }

        sprintf(custom_icon_path, "ux0:data/SMLA00001/icons/%s.png", game->id);
        if (tex.id == no_icon.id && FS::FileExists(custom_icon_path))
        {
            if (Textures::LoadImageFile(custom_icon_path, &tex))
            {
                game->tex = tex;
            }
        }
    }

    void Exit() {
    }

	void StartLoadImagesThread(int category, int prev_page_num, int page)
	{
		LoadImagesParams page_param;
        page_param.category = category;
		page_param.prev_page_num = prev_page_num;
		page_param.page_num = page;
		load_images_thid = sceKernelCreateThread("load_images_thread", (SceKernelThreadEntry)GAME::LoadImagesThread, 0x10000100, 0x4000, 0, 0, NULL);
		if (load_images_thid >= 0)
			sceKernelStartThread(load_images_thid, sizeof(LoadImagesParams), &page_param);
	}

    int LoadImagesThread(SceSize args, LoadImagesParams *params) {
        sceKernelDelayThread(300000);
        GAME::LoadGameImages(params->category, params->prev_page_num, params->page_num);
        return sceKernelExitDeleteThread(0);
    }

    void StartScanGamesThread()
    {
        scan_games_thid = sceKernelCreateThread("load_images_thread", (SceKernelThreadEntry)GAME::ScanGamesThread, 0x10000100, 0x4000, 0, 0, NULL);
		if (scan_games_thid >= 0)
			sceKernelStartThread(scan_games_thid, 0, NULL);
    }

    int ScanGamesThread(SceSize args, void *argp)
    {
        game_scan_complete = false;
        sceKernelDelayThread(500000);
        for (int i=0; i < TOTAL_CATEGORY; i++)
        {
            game_categories[i].games.clear();
        }

        GAME::Scan();
        game_scan_complete = true;

        if (game_categories[FAVORITES].games.size() > 0)
        {
            current_category = &game_categories[FAVORITES];
        }
        current_category->page_num = 1;
        view_mode = current_category->view_mode;
        GAME::StartLoadImagesThread(current_category->id, 1, 1);
        return sceKernelExitDeleteThread(0);
    }

    int GameComparator(const void *v1, const void *v2)
    {
        const Game *p1 = (Game *)v1;
        const Game *p2 = (Game *)v2;
        int p1_len = strlen(p1->title);
        int p2_len = strlen(p2->title);
        int len = p1_len;
        if (p2_len < p1_len)
            len = p2_len;
        return strncmp(p1->title, p2->title, len);
    }

    void DeleteGamesImages(GameCategory *category)
    {
        for (int i=0; i < category->games.size(); i++)
        {
            Game *game = &category->games[i];
            if (game->tex.id != no_icon.id)
            {
                Textures::Free(&game->tex);
                game->tex = no_icon;
            }
        }
    }

    void SaveFavorites()
    {
        void* fd = FS::Create(FAVORITES_FILE);
        for (int i=0; i < game_categories[FAVORITES].games.size(); i++)
        {
            Game* game = &game_categories[FAVORITES].games[i];
            char line[512];
            sprintf(line, "%s||%s||%s||%s\n", game->id, game->title, game->category, game->icon_path);
            FS::Write(fd, line, strlen(line));
        }
        FS::Close(fd);
    }

    void LoadFavorites()
    {
        if (FS::FileExists(FAVORITES_FILE))
        {
            std::vector<char> game_buffer = FS::Load(FAVORITES_FILE);
            int position = 0;
            while (position < game_buffer.size())
            {
                Game game;
                sprintf(game.id, "%s", nextToken(game_buffer, position).c_str());
                sprintf(game.title, "%s", nextToken(game_buffer, position).c_str());
                sprintf(game.category, "%s", nextToken(game_buffer, position).c_str());
                sprintf(game.icon_path, "%s",  nextToken(game_buffer, position).c_str());
                game.tex = no_icon;
                game_categories[FAVORITES].games.push_back(game);
            }
        }
    }

    Game* FindGame(GameCategory *category, char* title_id)
    {
        for (int i=0; i < category->games.size(); i++)
        {
            if (strcmp(title_id, category->games[i].id) == 0)
            {
                return &category->games[i];
            }
        }
        return nullptr;
    }

    int FindGamePosition(GameCategory *category, char* title_id)
    {
        for (int i=0; i < category->games.size(); i++)
        {
            if (strcmp(title_id, category->games[i].id) == 0)
            {
                return i;
            }
        }
        return -1;
    }

    int RemoveGameFromCategory(GameCategory *category, char* title_id)
    {
        for (int i=0; i < category->games.size(); i++)
        {
            if (strcmp(title_id, category->games[i].id) == 0)
            {
                category->games.erase(category->games.begin()+i);
                return i;
            }
        }
        return -1;
    }

    void SortGames(GameCategory *category)
    {
        qsort(&category->games[0], category->games.size(), sizeof(Game), GameComparator);
    }

    void RefreshGames()
    {
        FS::Rm(GAME_LIST_FILE);
        StartScanGamesThread();
    }

    bool IsMatchPrefixes(const char* id, std::vector<std::string> &prefixes)
    {
        for (int i=0; i < prefixes.size(); i++)
        {
            if (strncmp(id, prefixes[i].c_str(), strlen(prefixes[i].c_str())) == 0)
                return true;
        }
        return false;
    }

    const char* GetGameCategory(const char *id)
    {
        if (IsMatchPrefixes(id, game_categories[VITA_GAMES].valid_title_ids))
        {
            return vita.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[PSP_GAMES].valid_title_ids) &&
                 strncmp(id, "PSPEMUCFW", 9) != 0)
        {
            return psp.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[PS1_GAMES].valid_title_ids))
        {
            return ps1.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[PS_MIMI_GAMES].valid_title_ids))
        {
            return psmini.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[PS_MOBILE_GAMES].valid_title_ids))
        {
            return psmobile.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[NES_GAMES].valid_title_ids))
        {
            return nes.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[SNES_GAMES].valid_title_ids))
        {
            return snes.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[GB_GAMES].valid_title_ids))
        {
            return gb.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[GBA_GAMES].valid_title_ids))
        {
            return gba.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[N64_GAMES].valid_title_ids))
        {
            return n64.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[PORT_GAMES].valid_title_ids))
        {
            return ports.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[ORIGINAL_GAMES].valid_title_ids))
        {
            return original.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[UTILITIES].valid_title_ids))
        {
            return utilities.c_str();
        }
        else if (IsMatchPrefixes(id, game_categories[EMULATORS].valid_title_ids))
        {
            return emulator.c_str();
        }
        else
        {
            return homebrew.c_str();
        }
    }
}
