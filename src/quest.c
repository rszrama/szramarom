/****************************************************************************
*  Automated Quest code written by Vassago of MOONGATE, moongate.ams.com    *
*  4000. Copyright (c) 1996 Ryan Addams, All Rights Reserved. Use of this   *
*  code is allowed provided you add a credit line to the effect of:         *
*  "Quest Code (c) 1996 Ryan Addams" to your logon screen with the rest     *
*  of the standard diku/rom credits. If you use this or a modified version  *
*  of this code, let me know via email: moongate@moongate.ams.com. Further  *
*  updates will be posted to the rom mailing list. If you'd like to get     *
*  the latest version of quest.c, please send a request to the above add-   *
*  ress. Quest Code v2.03. Please do not remove this notice from this file. *
****************************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "merc.h"

DECLARE_DO_FUN( do_say );

/* Object vnums for Quest Rewards */

#define QUEST_ITEM1 3390
#define QUEST_ITEM2 3391

/* Object vnums for object quest 'tokens'. */

#define QUEST_OBJQUEST1 3380
#define QUEST_OBJQUEST2 3381
#define QUEST_OBJQUEST3 3382

/* Local functions */

void generate_quest     args(( CHAR_DATA *ch, CHAR_DATA *questman ));
void quest_update       args(( void ));
bool quest_level_diff   args(( int clevel, int mlevel));
bool chance             args(( int num ));

ROOM_INDEX_DATA         *find_location( CHAR_DATA *ch, char *arg );

/* CHANCE function. I use this everywhere in my code, very handy :> */

bool chance(int num)
{
    if (number_range(1, 100) <= num) return TRUE;
    else return FALSE;
}

/* The main quest function */

void do_quest(CHAR_DATA *ch, char *argument)
{
    CHAR_DATA *questman;
    OBJ_DATA *obj=NULL, *obj_next;
    OBJ_INDEX_DATA *questinfoobj;
    MOB_INDEX_DATA *questinfo;
    char buf [MAX_STRING_LENGTH];
    char arg1 [MAX_INPUT_LENGTH];
    char arg2 [MAX_INPUT_LENGTH];

    argument = one_argument(argument, arg1);
    argument = one_argument(argument, arg2);

    if (arg1[0] == '\0')
    {
        send_to_char("QUEST commands: POINTS INFO TIME REQUEST COMPLETE GIVEUP LIST BUY.\n\r",ch);
        send_to_char("For more information, type 'HELP QUEST'.\n\r",ch);
        return;
    }

    if (!strcmp(arg1, "info"))
    {
        if (IS_SET(ch->act, PLR_QUESTOR))
        {
            if (ch->questmob == -1 && ch->questgiver->short_descr != NULL)
            {
                sprintf(buf, "Your quest is ALMOST complete!\n\rGet back to %s before your time runs out!\n\r", ch->questgiver->short_descr);
                send_to_char(buf, ch);
            }
            else if (ch->questobj > 0)
            {
                questinfoobj = get_obj_index(ch->questobj);

                if (questinfoobj != NULL)
                {
                    sprintf(buf, "You are on a quest to recover the fabled %s!\n\r", questinfoobj->name);
                    send_to_char(buf, ch);
                }
                else
                {
                    send_to_char("You aren't currently on a quest.\n\r",ch);
                }

                return;
            }
            else if (ch->questmob > 0)
            {
                questinfo = get_mob_index(ch->questmob);

                if (questinfo != NULL)
                {
                    sprintf(buf, "You are on a quest to slay the dreaded %s!\n\r", questinfo->short_descr);
                    send_to_char(buf, ch);
                }
                else
                {
                    send_to_char("You aren't currently on a quest.\n\r", ch);
                }

                return;
            }
        }
        else
        {
            send_to_char("You aren't currently on a quest.\n\r", ch);
        }

        return;
    }

    if (!strcmp(arg1, "points"))
    {
        sprintf(buf, "{yYou currently have %d quest points to spend.{x\n\r", ch->qp_current);
        send_to_char(buf, ch);
        sprintf(buf, "In total you have earned %d quest points.\n\r", ch->qp_total);
        send_to_char(buf, ch);
        return;
    }
    else if (!strcmp(arg1, "time"))
    {
        if (!IS_SET(ch->act, PLR_QUESTOR))
        {
            send_to_char("You aren't currently on a quest.\n\r", ch);

            if (ch->nextquest > 1)
            {
                sprintf(buf, "There are %d minutes remaining until you can go on another quest.\n\r", ch->nextquest);
                send_to_char(buf, ch);
            }
            else if (ch->nextquest == 1)
            {
                sprintf(buf, "There is less than a minute remaining until you can go on another quest.\n\r");
                send_to_char(buf, ch);
            }
        }
        else if (ch->countdown > 0)
        {
            sprintf(buf, "Time left for current quest: %d\n\r", ch->countdown);
            send_to_char(buf, ch);
        }

        return;
    }

/* Checks for a character in the room with spec_questmaster set. This special
   procedure must be defined in special.c. You could instead use an
   ACT_QUESTMASTER flag instead of a special procedure. */

    for (questman = ch->in_room->people; questman != NULL; questman = questman->next_in_room)
    {
        if (!IS_NPC(questman))
        {
            continue;
        }

        if (questman->spec_fun == spec_lookup( "spec_questmaster" ))
        {
            break;
        }
    }

    if (questman == NULL || questman->spec_fun != spec_lookup( "spec_questmaster" ))
    {
        send_to_char("You can't do that here.\n\r", ch);
        return;
    }

    if (questman->fighting != NULL)
    {
        send_to_char("Wait until the fighting stops.\n\r", ch);
        return;
    }

    ch->questgiver = questman;

/* And, of course, you will need to change the following lines for YOUR
   quest item information. Quest items on Moongate are unbalanced, very
   very nice items, and no one has one yet, because it takes awhile to
   build up quest points :> Make the item worth their while. */

    if (!strcmp(arg1, "list"))
    {
        act("$n asks $N for a list of quest items.", ch, NULL, questman, TO_ROOM);
        act("You ask $N for a list of quest items.", ch, NULL, questman, TO_CHAR);

        sprintf(buf, "\n\r{yCurrent Quest Items available for Purchase:{x\n\r\n\r\
1250qp.........Amulet of Mota\n\r\
1000qp.........Bag of Tricks\n\r\
750qp..........10 Practices\n\r\
500qp..........1,000 gold pieces\n\r\n\r\
{cTo buy an item, type 'QUEST BUY <item>'.{x\n\r");

        send_to_char(buf, ch);

        return;
    }
    else if (!strcmp(arg1, "buy"))
    {
        if (arg2[0] == '\0')
        {
            send_to_char("To buy an item, type 'QUEST BUY <item>'.\n\r", ch);
            return;
        }

        if (is_name(arg2, "amulet"))
        {
            if (ch->qp_current >= 1250)
            {
                ch->qp_current -= 1250;
                obj = create_object(get_obj_index(QUEST_ITEM1), ch->level);
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", ch->name);
                do_say(questman, buf);
                return;
            }
        }
        else if (is_name(arg2, "bag tricks"))
        {
            if (ch->qp_current >= 1000)
            {
                ch->qp_current -= 1000;
                obj = create_object(get_obj_index(QUEST_ITEM2), ch->level);
            }
            else {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", ch->name);
                do_say(questman, buf);
                return;
            }
        }
        else if (is_name(arg2, "practices pracs prac practice"))
        {
            if (ch->qp_current >= 750)
            {
                ch->qp_current -= 750;
                ch->practice += 10;
                act("$N awards 10 practices to $n.", ch, NULL, questman, TO_ROOM);
                act("$N awards you 10 practices.", ch, NULL, questman, TO_CHAR);
                return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", ch->name);
                do_say(questman, buf);
                return;
            }
        }
        else if (is_name(arg2, "gold gp"))
        {
            if (ch->qp_current >= 500)
            {
                ch->qp_current -= 500;
                ch->gold += 1000;
                act("$N gives 1,000 gold pieces to $n.", ch, NULL, questman, TO_ROOM);
                act("$N hands over a cool 1,000 gold pieces.", ch, NULL, questman, TO_CHAR);
                return;
            }
            else
            {
                sprintf(buf, "Sorry, %s, but you don't have enough quest points for that.", ch->name);
                do_say(questman, buf);
                return;
            }
        }
        else
        {
            sprintf(buf, "I don't have that item, %s.", ch->name);
            do_say(questman, buf);
        }

        if (obj != NULL)
        {
            act("$N gives $p to $n.", ch, obj, questman, TO_ROOM);
            act("$N gives you $p.", ch, obj, questman, TO_CHAR);
            obj_to_char(obj, ch);
        }

        return;
    }
    else if (!strcmp(arg1, "request"))
    {
        act("$n asks $N for a quest.", ch, NULL, questman, TO_ROOM);
        act("You ask $N for a quest.", ch, NULL, questman, TO_CHAR);

        if (IS_SET(ch->act, PLR_QUESTOR))
        {
            sprintf(buf, "But you're already on a quest!");
            do_say(questman, buf);
            return;
        }

        if (ch->nextquest > 0)
        {
            sprintf(buf, "You're very brave, %s, but let someone else have a chance.", ch->name);
            do_say(questman, buf);
            sprintf(buf, "Come back later.");
            do_say(questman, buf);
            return;
        }

        sprintf(buf, "Thank you, brave %s!", ch->name);
        do_say(questman, buf);
        ch->questmob = 0;
        ch->questobj = 0;

        generate_quest(ch, questman);

        if (ch->questmob > 0 || ch->questobj > 0)
        {
            ch->countdown = number_range(10, 30);
            SET_BIT(ch->act, PLR_QUESTOR);
            sprintf(buf, "You have %d minutes to complete this quest.", ch->countdown);
            do_say(questman, buf);
            sprintf(buf, "May the gods go with you!");
            do_say(questman, buf);
        }

        return;
    }
    else if (!strcmp(arg1, "complete"))
    {
        act("$n informs $N $e has completed $s quest.", ch, NULL, questman, TO_ROOM);
        act("You inform $N you have completed your quest.", ch, NULL, questman, TO_CHAR);

        if (ch->questgiver != questman)
        {
            sprintf(buf, "I never sent you on a quest! Perhaps you're thinking of someone else.");
            do_say(questman, buf);
            return;
        }

        if (IS_SET(ch->act, PLR_QUESTOR))
        {
            if (ch->questmob == -1 && ch->countdown > 0)
            {
                int reward, pointreward, pracreward;

                reward = number_range(25, 75);
                pointreward = number_range(15, 50);

                sprintf(buf, "Congratulations on completing your quest!");
                do_say(questman,buf);
                sprintf(buf,"As a reward, I am giving you %d quest points and %d gold.", pointreward, reward);
                do_say(questman,buf);

                if (chance(15))
                {
                    pracreward = number_range(1, 3);
                    sprintf(buf, "You gain %d practices!\n\r", pracreward);
                    send_to_char(buf, ch);
                    ch->practice += pracreward;
                }

                REMOVE_BIT(ch->act, PLR_QUESTOR);
                ch->questgiver = NULL;
                ch->countdown = 0;
                ch->questmob = 0;
                ch->questobj = 0;
                ch->nextquest = 10;
                ch->gold += reward;
                ch->qp_current += pointreward;
                ch->qp_total += pointreward;

                return;
            }
            else if (ch->questobj > 0 && ch->countdown > 0)
            {
                bool obj_found = FALSE;

                for (obj = ch->carrying; obj != NULL; obj= obj_next)
                {
                    obj_next = obj->next_content;

                    if (obj != NULL && obj->pIndexData->vnum == ch->questobj)
                    {
                        obj_found = TRUE;
                        break;
                    }
                }

                if (obj_found == TRUE)
                {
                    int reward, pointreward, pracreward;

                    reward = number_range(25, 75);
                    pointreward = number_range(15, 50);

                    act("You hand $p to $N.", ch, obj, questman, TO_CHAR);
                    act("$n hands $p to $N.", ch, obj, questman, TO_ROOM);

                    sprintf(buf, "Congratulations on completing your quest!");
                    do_say(questman, buf);
                    sprintf(buf, "As a reward, I am giving you %d quest points and %d gold.", pointreward, reward);
                    do_say(questman, buf);

                    if (chance(15))
                    {
                        pracreward = number_range(1, 3);
                        sprintf(buf, "You gain %d practices!\n\r", pracreward);
                        send_to_char(buf, ch);
                        ch->practice += pracreward;
                    }

                    REMOVE_BIT(ch->act, PLR_QUESTOR);
                    ch->questgiver = NULL;
                    ch->countdown = 0;
                    ch->questmob = 0;
                    ch->questobj = 0;
                    ch->nextquest = 10;
                    ch->gold += reward;
                    ch->qp_current += pointreward;
                    ch->qp_total += pointreward;
                    extract_obj(obj);

                    return;
                }
                else
                {
                    sprintf(buf, "You haven't completed the quest yet, but there is still time!");
                    do_say(questman, buf);
                    return;
                }

                return;
            }
            else if ((ch->questmob > 0 || ch->questobj > 0) && ch->countdown > 0)
            {
                sprintf(buf, "You haven't completed the quest yet, but there is still time!");
                do_say(questman, buf);
                return;
            }
        }

        if (ch->nextquest > 0)
        {
            sprintf(buf, "But you didn't complete your quest in time!");
        }
        else
        {
            sprintf(buf, "You have to REQUEST a quest first, %s.", ch->name);
        }

        do_say(questman, buf);
        return;
    }
    else if (!strcmp(arg1, "giveup"))
    {
        act("$n asks $N to give up $s quest.", ch, NULL, questman, TO_ROOM);
        act("You ask $N to giveup your quest.", ch, NULL, questman, TO_CHAR);

        if (ch->nextquest > 0)
        {
            sprintf(buf, "You're not on a quest, %s. Request one first.", ch->name);
            do_say(questman, buf);
            return;
        }

        if (IS_SET(ch->act, PLR_QUESTOR))
        {
            sprintf(buf, "I'm disappointed in you, %s. You can redeem yourself in a little while.", ch->name);
            do_say(questman, buf);

            REMOVE_BIT(ch->act, PLR_QUESTOR);
            ch->questgiver = NULL;
            ch->countdown = 0;
            ch->questmob = 0;
            ch->questobj = 0;
            ch->nextquest = 15;

            return;
        }
        else
        {
            send_to_char("You aren't currently on a quest.\n\r", ch);
            return;
        }
   }

    send_to_char("QUEST commands: POINTS INFO TIME REQUEST COMPLETE GIVEUP LIST BUY.\n\r", ch);
    send_to_char("For more information, type 'HELP QUEST'.\n\r", ch);

    return;
}

void generate_quest(CHAR_DATA *ch, CHAR_DATA *questman)
{
    CHAR_DATA *victim;
    ROOM_INDEX_DATA *room;
    OBJ_DATA *questitem;
    char buf [MAX_STRING_LENGTH];

    /*  Randomly selects a mob from the world mob list. If you don't
        want a mob to be selected, make sure it is immune to summon.
        Or, you could add a new mob flag called ACT_NOQUEST. The mob
        is selected for both mob and obj quests, even tho in the obj
        quest the mob is not used. This is done to assure the level
        of difficulty for the area isn't too great for the player. */

    for (victim = char_list; victim != NULL; victim = victim->next)
    {
        if (!IS_NPC(victim))
        {
            continue;
        }

        if (quest_level_diff(ch->level, victim->level) == TRUE
            && !IS_SET(victim->imm_flags, IMM_SUMMON)
            && victim->pIndexData != NULL
            && victim->pIndexData->pShop == NULL
            && !IS_SET(victim->act, ACT_PET)
            && !IS_SET(victim->affected_by, AFF_CHARM)
            && chance(15))
        {
            break;
        }
    }

    if (victim == NULL)
    {
        sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
        do_say(questman, buf);
        sprintf(buf, "Try again later.");
        do_say(questman, buf);
        ch->nextquest = 2;
        return;
    }

    if ((room = find_location(ch, victim->name)) == NULL)
    {
        sprintf(buf, "I'm sorry, but I don't have any quests for you at this time.");
        do_say(questman, buf);
        sprintf(buf, "Try again later.");
        do_say(questman, buf);
        ch->nextquest = 2;
        return;
    }

    /*  40% chance it will send the player on a 'recover item' quest. */

    if (chance(40))
    {
        int objvnum = 0;

        switch(number_range(0, 2))
        {
            case 0:
            objvnum = QUEST_OBJQUEST1;
            break;

            case 1:
            objvnum = QUEST_OBJQUEST2;
            break;

            case 2:
            objvnum = QUEST_OBJQUEST3;
            break;
        }

        questitem = create_object(get_obj_index(objvnum), ch->level);
        obj_to_room(questitem, room);
        ch->questobj = questitem->pIndexData->vnum;

        sprintf(buf, "Vile pilferers have stolen %s from the royal treasury!", questitem->short_descr);
        do_say(questman, buf);
        do_say(questman, "My court wizardess, with her magic mirror, has pinpointed its location.");

        sprintf(buf, "Look in the general area of %s for %s!", room->area->name, room->name);
        do_say(questman, buf);
        return;
    }

    /* Quest to kill a mob */

    else
    {
        switch(number_range(0, 1))
        {
            case 0:
            sprintf(buf, "An enemy of mine, %s, is making vile threats against the crown.", victim->short_descr);
            do_say(questman, buf);
            sprintf(buf, "This threat must be eliminated!");
            do_say(questman, buf);
            break;

            case 1:
            sprintf(buf, "Rune's most heinous criminal, %s, has escaped from the dungeon!", victim->short_descr);
            do_say(questman, buf);
            sprintf(buf, "Since the escape, %s has murdered %d civilians!", victim->short_descr, number_range(2, 20));
            do_say(questman, buf);
            do_say(questman, "The penalty for this crime is death, and you are to deliver the sentence!");
            break;
        }

        if (room->name != NULL)
        {
            sprintf(buf, "Seek %s out somewhere in the vicinity of %s!", victim->short_descr, room->name);
            do_say(questman, buf);

            sprintf(buf, "That location is in the general area of %s.", room->area->name);
            do_say(questman, buf);
        }

        ch->questmob = victim->pIndexData->vnum;
    }
    return;
}

bool quest_level_diff(int clevel, int mlevel)
{
    if (clevel < 9 && mlevel < clevel + 2) return TRUE;
    else if (clevel <= 9 && mlevel < clevel + 3
	  && mlevel > clevel - 5) return TRUE;
    else if (clevel <= 14 && mlevel < clevel + 4
	  && mlevel > clevel - 5) return TRUE;
    else if (clevel <= 21 && mlevel < clevel + 5
	  && mlevel > clevel - 4) return TRUE;
    else if (clevel <= 29 && mlevel < clevel + 6
	  && mlevel > clevel - 3) return TRUE;
    else if (clevel <= 37 && mlevel < clevel + 7
	  && mlevel > clevel - 2) return TRUE;
    else if (clevel <= 55 && mlevel < clevel + 8
	  && mlevel > clevel - 1) return TRUE;
    else if(clevel > 55) return TRUE; /* Imms can get anything :) */
    else return FALSE;
}

/* Called from update_handler() by pulse_area */

void quest_update(void)
{
    DESCRIPTOR_DATA *d;
    CHAR_DATA *ch;

    for (d = descriptor_list; d != NULL; d = d->next)
    {
        if (d->character != NULL && d->connected == CON_PLAYING)
        {
            ch = d->character;

            if (ch->nextquest > 0)
            {
                if (--ch->nextquest == 0)
                {
                    send_to_char("{MYou may now quest again.{x\n\r", ch);
                    return;
                }
            }
            else if (IS_SET(ch->act, PLR_QUESTOR))
            {
                if (--ch->countdown <= 0)
                {
                    char buf [MAX_STRING_LENGTH];

                    ch->nextquest = 10;
                    sprintf(buf, "{MYou have run out of time for your quest!{x\n\r{MYou may quest again in %d minutes.{x\n\r", ch->nextquest);
                    send_to_char(buf, ch);
                    REMOVE_BIT(ch->act, PLR_QUESTOR);
                    ch->questgiver = NULL;
                    ch->countdown = 0;
                    ch->questmob = 0;

                    // @todo destory the quest item.
                }
                if (ch->countdown > 0 && ch->countdown < 6)
                {
                    send_to_char("{MBetter hurry, you're almost out of time for your quest!{x\n\r", ch);
                    return;
                }
            }
        }
    }

    return;
}
