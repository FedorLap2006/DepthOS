#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

const char *frame0[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ",,>>>>>>>>,,,,,,,,>>>>>>'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    ">>&&&&&&&&>>>>>>>>&&&&&'@@$$$$$-$$-$$$$@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$$''$-$$@','',,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$$$$$$$'**'$$$@''**',,,,,,,,,,,,,,,,,,",
    "&&++++++++&&&&&&&&'''++'@$$$$$-$$'***$$$@'***',,,,,,,,,,,,,,,,,,",
    "++++++++++++++++++**''+'@$$$$$$$$'***''''****',,,,,,,,,,,,,,,,,,",
    "++++++++++++++++++'**'''@$$$$$$$$'***********',,,,,,,,,,,,,,,,,,",
    "++########++++++++''**''@$$$$$$-'*************',,,,,,,,,,,,,,,,,",
    "###################''**'@$-$$$$$'***.'****.'**',,,,,,,,,,,,,,,,,",
    "####################''''@$$$$$$$'***''**'*''**',,,,,,,,,,,,,,,,,",
    "##========########====''@@$$$-$$'*%%********%%',,,,,,,,,,,,,,,,,",
    "======================='@@@$$$$$$'***''''''**',,,,,,,,,,,,,,,,,,",
    "==;;;;;;;;.=======;;;;'''@@@@@@@@@'*********',,,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;'***''''''''''''''''''',,,,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;'**'','*',,,,,'*','**',,,,,,,,,,,,,,,,,,,,,",
    ";;,,,,,.,,;;;.;;;;,,,'''',,'',,,,,,,'',,'',,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char *frame1[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ",,>>>>>>>>,,,,,,,,>>>>>>'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    ">>&&&&&&&&>>>>>>>>&&&&&'@@$$$$$-$$-$$$$@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$$$''-$$@',,'',,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$$$$$$$$'**'$$@','**',,,,,,,,,,,,,,,,,",
    "&&++++++++&&&&&&&&+++++'@$$$$$-$$$'***$$@''***',,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++'+++'@$$$$$$$$$'***''''****',,,,,,,,,,,,,,,,,",
    "++++++++++++++++++'*'++'@$$$$$$$$$'***********',,,,,,,,,,,,,,,,,",
    "++########++++++++'*''''@$$$$$$-$'*************',,,,,,,,,,,,,,,,",
    "###################****'@$-$$$$$$'***.'****.'**',,,,,,,,,,,,,,,,",
    "###################''**'@$$$$$$$$'***''**'*''**',,,,,,,,,,,,,,,,",
    "##========########==='''@@$$$-$$$'*%%********%%',,,,,,,,,,,,,,,,",
    "======================='@@@$$$$$$$'***''''''**',,,,,,,,,,,,,,,,,",
    "==;;;;;;;;========;;;;;''@@@@@@@@@@'*********',,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;;'**'''''''''''''''''''',,,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;;'**','*',,,,,,**','**',,,,,,,,,,,,,,,,,,,,",
    ";;,,,.,,,,;;;;;;;;,,,,''',,,'',,,,,,''',,''',,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char *frame2[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,.",
    ">>,,,,,,,>>>>>>>>,,,,,,,,''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>>'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    "&&>>>>>>>&&&&&&&&>>>>>>'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@@$$$$$-$$-$$$$@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$$$''-$$@',,'',,,,,,,,,,,,,,,,,,",
    "++&&&&&&&++++++++&&&&&&'@$$$$$$$$$'**'$$@','**',,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++++++'@$$$$$-$$$'***$$@''***',,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++++++'@$$$$$$$$$'***''''****',,,,,,,,,,,,,,,,,",
    "##+++++++########++++++'@$$$$$$$$$'***********',,,,,,,,,,,,,,,,,",
    "######################''@$$$$$$-$'*************',,,,,,,,,,,,,,,,",
    "###################'''''@$-$$$$$$'***.'****.'**',,,,,,,,,,,,,,,,",
    "==#######========#'****'@$$$$$$$$'***''**'*''**',,,,,,,,,,,,,,,,",
    "==================='''='@@$$$-$$$'*%%********%%',,,,,,,,,,,,,,,,",
    ";;=======;;;;;;;;======'@@@$$$$$$$'***''''''**',,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;;;''@@@@@@@@@@'*********',,,,,,,,,,,,,,,,,,",
    ";.;;;;;;;;;;;;;;;;;;;;;'*'''''''''''''''''''',,,,,,,,,,,,,,,,,,,",
    ".,.;;;;;;,,,,,,,,;;;;;;'**',**',,,,,,**','**',,,,,,,,,,,,,,,,,,,",
    ",.,,,,,,,,,,,,,,,,,,,,,''',,''',,,,,,''',,''',,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char *frame3[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,.,,,,,,,",
    ">>,,,,,,,>>>>>>>>,,,,,,,,''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>>'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    "&&>>>>>>>&&&&&&&&>>>>>>'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@@$$$$$-$$-$$$$@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$$$''-$$@',,'',,,,,,,,,,,,,,,,,,",
    "++&&&&&&&++++++++&&&&&&'@$$$$$$$$$'**'$$@','**',,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++++++'@$$$$$-$$$'***$$@''***',,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++++++'@$$$$$$$$$'***''''****',,,,,,,,,,,,,,,,,",
    "##+++++++########++++++'@$$$$$$$$$'***********',,,,,,,,,,,,,,,,,",
    "#####################'''@$$$$$$-$'*************',,,,,,,,,,,,,,,,",
    "###################''**'@$-$$$$$$'***.'****.'**',,,,,,,,,,,,,,,,",
    "==#######========##****'@$$$$$$$$'***''**'*''**',,,,,,,,,,,,,,,,",
    "=================='*'=='@@$$$-$$$'*%%********%%',,,,,,,,,,,,,,,,",
    ";;=======;;;;;;;;=='==='@@@$$$$$$$'***''''''**',,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;;;''@@@@@@@@@@'*********',,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;;'**'''''''''''''''''''',,,,,,,,,,,,,,,,,,,",
    ",,;;;;;;;,,,,,,,,;;;;;'**','*',,,,,,'*','**',,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,''',,,'',,,,,,,'',,''',,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char *frame4[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,.,,,,,,,,,,,,,,",
    ",,>>>>>>>>,,,,,,,,>>>>>>>''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>>'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    ">>&&&&&&&&>>>>>>>>&&&&&'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@@$$$$$-$$-$$$$@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$$''$-$$@','',,,,,,,,,,,,,,,,,,,",
    "&&++++++++&&&&&&&&+++++'@$$$$$$$$'**'$$$@''**',,,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++++++'@$$$$$-$$'***$$$@'***',,,,,,,,,,,,,,,,,,",
    "++++++++++++++++++'''++'@$$$$$$$$'***''''****',,,,,,,,,,,,,,,,,,",
    "++########+++++++'**''''@$$$$$$$$'***********',,,,,,,,,,,,,,,,,,",
    "#################'****''@$$$$$$-'*************',,,,,,,,,,,,,,,,,",
    "##################''''*'@$-$$$$$'***.'****.'**',,,,,,,,,,,,,,,,,",
    "##========########==='''@$$$$$$$'***''**'*''**',,,,,,,,,,,,,,,,,",
    "======================='@@$$$-$$'*%%********%%',,,,,,,,,,,,,,,,,",
    "==;;;;;;;;========;;;;''@@@$$$$$$'***''''''**',,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;''''@@@@@@@@@'*********',,,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;'***'''''''''''''''''''',,,,,,,,,,,,,,,,,,,,",
    ";;,,,,,,,,;;;;;;;;,,'**','**,,,,,,'**,'**',,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,''',,,'',,,,,,,'',,''',,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char *frame5[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,>>>>>>>>,,,,,,,,>>>>>>>''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>>'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    ">>&&&&&&&&>>>>>>>>&&&&&'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@@$$$$$-$$''$$$@@','',,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$'**'-$$@''**',,,,,,,,,,,,,,,,,,",
    "&&++++++++&&&&&&&&+++++'@$$$$$$$$'***$$$@'***',,,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++'+++'@$$$$$-$$'***''''****',,,,,,,,,,,,,,,,,,",
    "++++++++++++++++++'*'++'@$$$$$$$$'***********',,,,,,,,,,,,,,,,,,",
    "++########++++++++'*''''@$$$$$$$'*************',,,,,,,,,,,,,,,,,",
    "###################****'@$$$$$$-'***.'****.'**',,,,,,,,,,,,,,,,,",
    "###################''**'@$-$$$$$'***''**'*''**',,,,,,,,,,,,,,,,,",
    "##========########==='''@$$$$$$$'*%%********%%',,,,,,,,,,,,,,,,,",
    "======================='@@$$$-$$$'***''''''**',,,,,,,,,,,,,,,,,,",
    "==;;;;;;;;========;;;;''@@@$$$$$$$'*********',,,,,,,,,,,,,,,,,,.",
    ";;;;;;;;;;;;;;;;;;;;;'*''@@@@@@@@@@''''''''',,,,,,,,,,,,,,,,,,,.",
    ";;;;;;;;;;;;;;;;;;;;'***''''''''''''''''*',,,,,,,,,,,,,,,,,,,,,,",
    ";;,,,,,,,,;;;;;;;;,,'**','**,,,,,,'**,'**',,,,,,,,,,,,,,,,,,..,.",
    ",,,,,,,,,,,,,,,,,,,,''',,''',,,,,,''',,''',,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,.",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char *frame6[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ">>,,,,,,,>>>>>>>>,,,,,,,'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&>>>>>>>&&&&&&&&>>>>>>'@@$$$$$-$$-$$$$@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$$''$-$$@','',,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$$$$$$$'**'$$$@''**',,,,,,,,,,,,,,,,,,",
    "++&&&&&&&++++++++&'''&&'@$$$$$-$$'***$$$@'***',,,,,,,,,,,,,,,,,,",
    "++++++++++++++++++'*''+'@$$$$$$$$'***''''****',,,,,,,,,,,,,,,,,,",
    "++++++++++++++++++'**'''@$$$$$$$$'***********',,,,,,,,,,,,,,,,,,",
    "##+++++++########++'**''@$$$$$$-'*************',,,,,,,,,,,,,,,,,",
    "###################''**'@$-$$$$$'***.'****.'**',,,,,,,,,,,,,,,,,",
    "####################''''@$$$$$$$'***''**'*''**',,,,,,,,,,,,,,,,,",
    "==#######========#####''@@$$$-$$'*%%********%%',,,,,,,,,,,,,,,,,",
    "======================='@@@$$$$$$'***''''''**',,,,,,,,,,,,,,,,,,",
    ";;=======;;;;;;;;====='''@@@@@@@@@'*********',,,,,,,,,,,.,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;'***''''''''''''''''''',,,,,,,,,,.,,,.,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;'**'','*',,,,,'**,'**',,,,,,,,,,,,,,,,,,,,,",
    ",,;;;;;;;,,,,,,,,;;;;'''',,'',,,,,,,'',,'',,,,,,,,,,,.,,,,,.,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,.,,,.,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char *frame7[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ">>,,,,,,,>>>>>>>>,,,,,,,'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&>>>>>>>&&&&&&&&>>>>>>'@@$$$$$-$$-$$$$@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$$$''-$$@',,'',,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$$$$$$$$'**'$$@','**',,,,,,,,,,,,,,,,,",
    "++&&&&&&&++++++++&&&&&&'@$$$$$-$$$'***$$@''***',,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++'+++'@$$$$$$$$$'***''''****',,,,,,,,,,,,,,,,,",
    "++++++++++++++++++'*'++'@$$$$$$$$$'***********',,,,,,,,,,,,,,,,,",
    "##+++++++########+'*''''@$$$$$$-$'*************',,,,,,,,,,,,,,,,",
    "###################****'@$-$$$$$$'***.'****.'**',,,,,,,,,,,,,,,,",
    "###################''**'@$$$$$$$$'***''**'*''**',,,,,,,,,,,,,,,,",
    "==#######========####'''@@$$$-$$$'*%%********%%',,,,,,,,,,,,,,,,",
    "======================='@@@$$$$$$$'***''''''**',,,,,,,,,,,,,,,,,",
    ";;=======;;;;;;;;======''@@@@@@@@@@'*********',,,.,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;;'**'''''''''''''''''''',,,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;;'**','*',,,,,,**','**',,,,,,,,,,,,,,,,,,,,",
    ",,;;;;;;;,,,,,,,,;;;;;''',,,'',,,,,,''',,''',,.,,,,.,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char *frame8[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,.,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,>>>>>>>>,,,,,,,,>>>>>>>''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>>'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    ">>&&&&&&&&>>>>>>>>&&&&&'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@@$$$$$-$$-$$$$@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$$$''-$$@',,'',,,,,,,,,,,,,,,,,,",
    "&&++++++++&&&&&&&&+++++'@$$$$$$$$$'**'$$@','**',,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++++++'@$$$$$-$$$'***$$@''***',,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++++++'@$$$$$$$$$'***''''****',,,,,,,,,,,,,,,,,",
    "++########++++++++#####'@$$$$$$$$$'***********',,,,,,,,,,,,,,,,,",
    "######################''@$$$$$$-$'*************',,,,,,,,,,,,,,,,",
    "###################'''''@$-$$$$$$'***.'****.'**',,,,,,,,,,,,,,,,",
    "##========########'****'@$$$$$$$$'***''**'*''**',,,,,,,,,,,,,,,,",
    "==================='''='@@$$$-$$$'*%%********%%',,,,,,,,,,,,,,,,",
    "==;;;;;;;;========;;;;;'@@@$$$$$$$'***''''''**',,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;;;''@@@@@@@@@@'*********',,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;;;'*'''''''''''''''''''',,,,,,,,,,,,,,,,,,,",
    ";;,,,,,,,,;;;;;;;;,,,,,'**',**',,,,,,**'.'**',,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,''',,''',,,,,,''',,''',,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char *frame9[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,.,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,>>>>>>>>,,,,,,,,>>>>>>>''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>>'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    ">>&&&&&&&&>>>>>>>>&&&&&'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@@$$$$$-$$-$$$$@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$$$''-$$@',,'',,,,,,,,,,,,,,,,,,",
    "&&++++++++&&&&&&&&+++++'@$$$$$$$$$'**'$$@','**',,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++++++'@$$$$$-$$$'***$$@''***',,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++++++'@$$$$$$$$$'***''''****',,,,,,,,,,,,,,,,,",
    "++########++++++++#####'@$$$$$$$$$'***********',,,,,,,,,,,,,,,,,",
    "#####################'''@$$$$$$-$'*************',,,,,,,,,,,,,,,,",
    "###################''**'@$-$$$$$$'***.'****.'**',,,,,,,,,,,,,,,,",
    "##========########=****'@$$$$$$$$'***''**'*''**',,,,,,,,,,,,,,,,",
    "=================='*'=='@@$$$-$$$'*%%********%%',,,,,,,,,,,,,,,,",
    "==;;;;;;;;========;';;;'@@@$$$$$$$'***''''''**',,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;;;''@@@@@@@@@@'*********',,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;;'**'''''''''''''''''''',,,,,,,,,,,,,,,,,,,",
    ";;,,,,,,,,;;;;;;;;,,,,'**','*',,..,.**','**',,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,''',,,'',,,,.,''',,''',,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,.,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char *frame10[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,.,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ">>,,,,,,,>>>>>>>>,,,,,,,,''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>>'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    "&&>>>>>>>&&&&&&&&>>>>>>'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@@$$$$$-$$-$$$$@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$$''$-$$@','',,,,,,,,,,,,,,,,,,,",
    "++&&&&&&&++++++++&&&&&&'@$$$$$$$$'**'$$$@''**',,,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++++++'@$$$$$-$$'***$$$@'***',,,,,,,,,,,,,,,,,,",
    "++++++++++++++++++'''++'@$$$$$$$$'***''''****',,,,,,,,,,,,,,,,,,",
    "##+++++++########'**''''@$$$$$$$$'***********',,,,,,,,,,,,,,,,,,",
    "#################'****''@$$$$$$-'*************',,,,,,,,,,,,,,,,,",
    "##################''''*'@$-$$$$$'***.'****.'**',,,,,,,,,,,,,,,,,",
    "==#######========####'''@$$$$$$$'***''**'*''**',,,,,,,,,,,,,,,,,",
    "======================='@@$$$-$$'*%%********%%',,,,,,,,,,,,,,,,,",
    ";;=======;;;;;;;;=====''@@@$$$$$$'***''''''**',,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;;''''@@@@@@@@@'*********',,,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;'***'''''''''''''''''''',,,,,,,,,,,,,,,,,,,,",
    ",,;;;;;;;,,,,,,,,;;;'**'.'**..,,,,'**''**',,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,''',,,'',,,,,,,''',''',,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,.,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char *frame11[] = {
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ">>,,,,,,,>>>>>>>>,,,,,,,,''''''''''''''',,,,,,,,,,,,,,,,,,,,,,,,",
    ">>>>>>>>>>>>>>>>>>>>>>>>'@@@@@@@@@@@@@@@',,,,,,,,,,,,,,,,,,,,,,,",
    "&&>>>>>>>&&&&&&&&>>>>>>'@@@$$$$$$$$$$$@@@',,,,,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@@$$$$$-$$''$$$@@','',,,,,,,,,,,,,,,,,,,",
    "&&&&&&&&&&&&&&&&&&&&&&&'@$$-$$$$$'**'-$$@''**',,,,,,,,,,,,,,,,,,",
    "++&&&&&&&++++++++&&&&&&'@$$$$$$$$'***$$$@'***',,,,,,,,,,,,,,,,,,",
    "+++++++++++++++++++'+++'@$$$$$-$$'***''''****',,,,,,,,,,,,,,,,,,",
    "++++++++++++++++++'*'++'@$$$$$$$$'***********',,,,,,,,,,,,,,,,,,",
    "##+++++++########+'*''''@$$$$$$$'*************',,,,,,,,,,,,,,,,,",
    "###################****'@$$$$$$-'***.'****.'**',,,,,,,,,,,,,,,,,",
    "###################''**'@$-$$$$$'***''**'*''**',,,,,,,,,,,,,,,,,",
    "==#######========####'''@$$$$$$$'*%%********%%',,,,,,,,,,,,,,,,,",
    "======================='@@$$$-$$$'***''''''**',,,,,,,,,,,,,,,,,,",
    ";;=======;;;;;;;;=.===''@@@$$$$$$$'*********',,,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;.'*''@@@@@@@@@@''''''''',,,,,,,,,,,,,,,,,,,,",
    ";;;;;;;;;;;;;;;;;;;;'***''''''''''''''''*',,,,,,,,,,,,,,,,,,,,,,",
    ",,;;;;;;;,,,,,,,.;;;'**','**,,,,,,'**''**',,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,''',,''',,,,,,''',,''',,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,.,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
    ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,",
};

const char **frames[] = {frame0,  frame1,  frame2, frame3, frame4,
                         frame5,  frame6,  frame7, frame8, frame9,
                         frame10, frame11, NULL};

#define FRAME_WIDTH 64
#define FRAME_HEIGHT 25

int main(int argc, char **argv) {
  int c = 0;
  while (1) {
    puts("\033[]");
    for (int i = 0; i < FRAME_HEIGHT; i++) {
      if (i)
        printf("\n");
      printf(frames[c][i]);
    }
    c++;
    c %= sizeof(frames) / sizeof(char *) - 1;
    usleep(500 * 1000);
    printf("\n");
  }
}
