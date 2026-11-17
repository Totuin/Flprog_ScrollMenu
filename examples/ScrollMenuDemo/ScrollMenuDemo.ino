#include <flprogScrollMenu.h>

String names[8] = {"Рецепт 1", "Рецепт 2", "Рецепт 3", "Рецепт 4", "Рецепт 5", "Рецепт 6", "Рецепт 7", "Рецепт 8"};
int16_t temperatures[8] = {1, 2, 3, 4, 5, 6, 7, 8}; // условные тестовые числа
int16_t times[8] = {10, 20, 30, 40, 50, 60, 70, 80};
String visibleNames[5];
int16_t visibleTemperatures[5], visibleTimes[5];
FlprogScrollMenu menu(5);
FlprogScrollMenuStringColumn nameColumn(8, 5);
FlprogScrollMenuInt16Column temperatureColumn(8, 5), timeColumn(8, 5);
FlprogAbstractScrollMenuColumn* columns[] = {&nameColumn, &temperatureColumn, &timeColumn};
String readName(uint16_t i) { return names[i]; }
int16_t readTemperature(uint16_t i) { return temperatures[i]; }
int16_t readTime(uint16_t i) { return times[i]; }
void writeName(String value, uint16_t i) { visibleNames[i] = value; }
void writeTemperature(int16_t value, uint16_t i) { visibleTemperatures[i] = value; }
void writeTime(int16_t value, uint16_t i) { visibleTimes[i] = value; }
void setup() {
    nameColumn.setReadFunction(readName); nameColumn.setWriteFunction(writeName);
    temperatureColumn.setReadFunction(readTemperature); temperatureColumn.setWriteFunction(writeTemperature);
    timeColumn.setReadFunction(readTime); timeColumn.setWriteFunction(writeTime);
    menu.setColumns(columns, 3);
    Serial.begin(115200);
}
void loop() {
    static int32_t index = 0;
    static int32_t previous = -2;
    static uint32_t last = 0;
    if (millis() - last >= 1500) { last = millis(); index = (index + 1) % 8; }
    menu.setCount(8); menu.setIndex(index);
    bool copied = menu.pool();
    if (!menu.isReady()) return;
    if (copied || previous != menu.getSelectedRow()) {
        previous = menu.getSelectedRow();
        for (uint16_t row = 0; row < menu.getVisibleRows(); ++row) {
            Serial.print(row == previous ? "> " : "  ");
            if (menu.isRowValid(row)) {
                Serial.print(visibleNames[row]); Serial.print(" | ");
                Serial.print(visibleTemperatures[row]); Serial.print(" | "); Serial.print(visibleTimes[row]);
            }
            Serial.println();
        }
    }
}
