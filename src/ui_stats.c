#include "ui_stats.h"
#include "ui_inventory.h"
#include "ui_ninepatch.h"

//temporary bullshit
void stats_ui_open (StatsUI *ui) { ui->open = 1; }
void stats_ui_close(StatsUI *ui) { ui->open = 0; }

void render_stats_screen(RenderCtx *ctx, const Player *p) {
    NinePatch window = chrome_window(ctx->ui.chrome);
    ninepatch_draw(ctx->renderer, &window, DESC_PANEL_X, DESC_PANEL_Y, DESC_PANEL_W, DESC_PANEL_H);
    
    /*label*/
    const char *class_name = "Unknown";

    switch (p->cls) {
        case HERO_WARRIOR:
            switch (p->sub_class) {
                case HERO_SUB_BERSERKER:  class_name = "Berserker";  break;
                case HERO_SUB_GLADIATOR:  class_name = "Gladiator";  break;
                default:                  class_name = "Warrior";    break;
            }
            break;
        case HERO_MAGE:
            switch (p->sub_class) {
                case HERO_SUB_WARLOCK:    class_name = "Warlock";    break;
                case HERO_SUB_BATTLEMAGE: class_name = "Battlemage"; break;
                default:                  class_name = "Mage";       break;
            }
            break;
        case HERO_ROGUE:
            switch (p->sub_class) {
                case HERO_SUB_FREERUNNER: class_name = "Freerunner"; break;
                case HERO_SUB_ASSASSIN:   class_name = "Assassin";   break;
                default:                  class_name = "Rogue";      break;
            }
            break;
        case HERO_HUNTRESS:
            switch (p->sub_class) {
                case HERO_SUB_SNIPER: class_name = "Sniper";  break;
                case HERO_SUB_WARDEN: class_name = "Warden";  break;
                default:              class_name = "Huntress"; break;
            }
            break;
        default:
            class_name = "Unknown";
            break;
    }

    int lvl = p->lvl;
    char current_lvl[64];
    snprintf(current_lvl, sizeof(current_lvl), "LEVEL %d %s", lvl, class_name);
    font_draw_text(ctx->renderer, &ctx->font, 160, 50, 1.6, COL_GOLD_TEXT, current_lvl);

    /*str*/
    int str = p->STR;
    char current_str[32];
    snprintf(current_str, sizeof(current_str), "Strength: %d", str);
    font_draw_text(ctx->renderer, &ctx->font, 160, 80, 1.5, COL_WHITE, current_str);

    /*hp*/
    int hp = p->base.HP;
    int max_hp = p->base.HT;

    if (hp < 0) hp = 0;
    if (hp > max_hp) hp = max_hp;

    char current_hp[32];
    snprintf(current_hp, sizeof(current_hp), "Health: %d/%d", hp, max_hp);
    font_draw_text(ctx->renderer, &ctx->font, 160, 95, 1.5, COL_WHITE, current_hp);

    /*exp*/
    int exp = p->exp;
    int max_exp = player_max_exp(p);

    if (exp < 0) exp = 0;
    if (exp > max_exp) exp = max_exp;

    char current_exp[32];
    snprintf(current_exp, sizeof(current_exp), "Experience: %d/%d", exp, max_exp);
    font_draw_text(ctx->renderer, &ctx->font, 160, 110, 1.5, COL_WHITE, current_exp);
}