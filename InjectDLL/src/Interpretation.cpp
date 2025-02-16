#include "Connectivity.h"
#include <iostream>

std::map<int, std::map<std::string, std::any>> Connectivity::convertData(std::string data, bool printData)
{
    std::map<int, std::map<std::string, std::any>> result;

    std::map<std::string, std::any> playerData;
    std::map<std::string, std::any> enemyData;

    //double Location[3] = { 0, 0, 0 };

    int startingPosition = 0;
    int count = 0;
    int playerCount = 0;
    int enemyHealthStart = 0;

    if(printData) system("CLS");

    for (int i = 0; i < data.size() + 1; i++)
    {

        if (playerCount == 4)
        {
            enemyHealthStart = i;
            break;
        }

        if (data[i] == ';')
        {
            if (data[i - 1] == '+')
            {
                startingPosition = i + 1;
                continue;
            }

            std::string substring;

            substring = data.substr(startingPosition, i - startingPosition);


            if (count == 0)
            {
                playerData["Name"] = substring;
                count++;
                if (printData) std::cout << "PLAYER " << playerCount + 1 << std::endl;
                if(printData) std::cout << "Name: " << substring << std::endl;
            }
            else if (count == 1)
            {
                playerData["Position_x"] = std::stod(substring);
                if(printData) std::cout << "Position: [x] = " << substring;
                count++;
            }
            else if (count == 2)
            {
                playerData["Position_y"] = std::stod(substring);
                if (printData) std::cout << " [y] = " << substring;
                count++;
            }
            else if (count == 3)
            {
                playerData["Position_z"] = std::stod(substring);
                if (printData) std::cout << " [z] = " << substring << std::endl;
                count++;
            }
            else if (count == 4)
            {

                playerData["Rotation"] = std::stod(substring);
                if (printData) std::cout << "Rotation: " << substring << std::endl;
                count++;

            }
            else if (count == 5)
            {
                int schd = std::stol(substring.c_str(), nullptr, 10);
                playerData["Schedule"] = schd;
                if (printData) std::cout << "Animation: [schd] = " << substring;
                count++;
            }
            else if (count == 6)
            {
                int anim = std::stol(substring.c_str(), nullptr, 10);
                playerData["Animation"] = anim;
                if (printData) std::cout << " [anim] = " << substring << std::endl;
                count++;
            }
            else if (count == 7)
            {
                playerData["Speed_x"] = std::stod(substring);
                if (printData) std::cout << "Speed: [x] = " << substring;
                count++;
            }
            else if (count == 8)
            {
                playerData["Speed_y"] = std::stod(substring);
                if (printData) std::cout << " [y] = " << substring;
                count++;
            }
            else if (count == 9)
            {
                playerData["Speed_z"] = std::stod(substring);
                if (printData) std::cout << " [z] = " << substring << std::endl << std::endl;
                count++;
            }
            else if (count == 10)
            {
                playerData["Map"] = substring;
                count++;
                if (printData) std::cout << " Map = " << substring << std::endl << std::endl;
            }
            else if (count == 11)
            {
                playerData["Section"] = substring;
                count++;
                if (printData) std::cout << " Section = " << substring << std::endl << std::endl;
            }
            else if (count == 12)
            {
                playerData["Town"] = substring;
                count = 0;
                result[playerCount] = playerData;
                playerCount++;
                if (printData) std::cout << " Town = " << substring << std::endl << std::endl;
                if (printData) std::cout << "---------------------------------------" << std::endl;
            }

            startingPosition = i + 1;
            //if (count == 0)
            //{
            //    startingPosition = startingPosition + 1;
            //}

        }
    }

    std::string ID;

    startingPosition--;

    if (printData) std::cout << "ENEMY DATA" << std::endl << std::endl;

    for (int i = enemyHealthStart; i < data.size() + 1; i++)
    {
        if (data[i] == ',' || data[i] == ' ')
        {
            //if (data[i + 1] == ' ')
            //{
            //    startingPosition = i + 1;
            //    continue;
            //}

            std::string substring;

            substring = data.substr(startingPosition, i - startingPosition);

            if (count == 0)
            {
                ID = substring;
                if (printData) std::cout << "[" << substring << "] = ";
                count++;
            }
            else if (count == 1)
            {
                int health = std::stol(substring.c_str(), nullptr, 10);
                if (printData) std::cout << substring << std::endl;
                enemyData[ID] = health;
                count = 0;
            }

            startingPosition = i + 1;
            /*if (count == 0)
            {
                startingPosition = startingPosition + 1;
            }*/

        }
    }

    result[4] = enemyData;

    return result;
}