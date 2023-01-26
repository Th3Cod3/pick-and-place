#ifndef _DELTA_ROBOT_DATA_H
#define _DELTA_ROBOT_DATA_H

#define STRING_LENGTH 2
#define BUFFER_SIZE sizeof(delta_robot_data_t)

typedef struct delta_robot_data_t {
    bool vacuum;
    int intNumber;
    float floatNumber;
    char string[STRING_LENGTH];
} delta_robot_data_t;

class DeltaRobotData {
private:
    delta_robot_data_t data;

public:
    DeltaRobotData(/* args */);
    byte* getLittleEndian();
    void printData();
};

DeltaRobotData::DeltaRobotData(/* args */)
{
}

byte* DeltaRobotData::getLittleEndian()
{
    byte* p_pointer[BUFFER_SIZE];
    // Zet adres van eerste byte van variabele in een byte-pointer
    byte* p_b = (byte*)&data.vacuum;
    byte* p_i = (byte*)&data.intNumber;
    byte* p_f = (byte*)&data.floatNumber;
    byte* p_c = (byte*)&data.string;

    // Vul buf met bool, bool, int, float, 2x char begin eind
    int j = 0;
    for (j = 0; j < 1; j++)
        *p_pointer[j] = *(p_b + j - 0); // j = 0 bool 1 byte in arduino j = 1
    for (j = j; j < 2; j++)
        *p_pointer[j] = *(p_b + j - 1); // j = 1 bool 2 bytes in plc j = 2
    for (j = j; j < 4; j++)
        *p_pointer[j] = *(p_i + j - 2); // j = 2 int want 2 bits j = 4
    for (j = j; j < 8; j++)
        *p_pointer[j] = *(p_f + j - 4); // j = 4 float/real 4 bits j = 8
    for (j = j; j < 10; j++)
        *p_pointer[j] = *(p_c + j - 8); // j = 8 2 characters j = 10

    return p_pointer[0];
}

void DeltaRobotData::printData()
{
    Serial.print(data.isTrue);
    Serial.print("\t|\t");
    Serial.print(data.isTrue, HEX);
    Serial.print("\t|\t");
    Serial.println(data.isTrue, BIN);

    Serial.print(data.intNumber);
    Serial.print("\t|\t");
    Serial.print(data.intNumber, HEX);
    Serial.print("\t|\t");
    Serial.print(data.intNumber, BIN);

    Serial.print(data.floatNumber);
    Serial.print("\t|\t");
    Serial.print(data.floatNumber, HEX);
    Serial.print("\t|\t");
    Serial.print(data.floatNumber, BIN);

    Serial.print(data.string);
    Serial.print("\t|\t");
    Serial.print((uint16_t) data.string, HEX);
    Serial.print("\t|\t");
    Serial.print((uint16_t) data.string, BIN);

}
#endif
