#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <cctype>

// Базовый интерфейс сериализатора
class Serializer {
public:
    virtual ~Serializer() = default;

    virtual void add_field(const std::string& name, const std::string& value) = 0;
    virtual void add_field(const std::string& name, int value) = 0;
    virtual void add_field(const std::string& name, double value) = 0;

    virtual void add_block(const std::string& name) = 0;
    virtual void end_block() = 0;

    virtual std::string build() = 0;
};

// XML сериализатор
class XmlSerializer : public Serializer {
private:
    std::vector<std::string> blocks_;
    std::string content_;
    int indent_level_ = 0;

    std::string get_indent() const {
        return std::string(indent_level_ * 2, ' ');
    }

public:
    void add_field(const std::string& name, const std::string& value) override {
        content_ += get_indent() + "<" + name + ">" + value + "</" + name + ">\n";
    }

    void add_field(const std::string& name, int value) override {
        content_ += get_indent() + "<" + name + ">" + std::to_string(value) + "</" + name + ">\n";
    }

    void add_field(const std::string& name, double value) override {
        content_ += get_indent() + "<" + name + ">" + std::to_string(value) + "</" + name + ">\n";
    }

    void add_block(const std::string& name) override {
        content_ += get_indent() + "<" + name + ">\n";
        blocks_.push_back(name);
        indent_level_++;
    }

    void end_block() override {
        if (!blocks_.empty()) {
            indent_level_--;
            std::string block_name = blocks_.back();
            blocks_.pop_back();
            content_ += get_indent() + "</" + block_name + ">\n";
        }
    }

    std::string build() override {
        while (!blocks_.empty()) {
            end_block();
        }
        return content_;
    }
};

// JSON сериализатор
class JsonSerializer : public Serializer {
private:
    std::vector<std::string> blocks_;
    std::string content_;
    bool needs_comma_ = false;
    int indent_level_ = 0;

    std::string get_indent() const {
        return std::string(indent_level_ * 2, ' ');
    }

    void handle_comma() {
        if (needs_comma_) {
            content_ += ",";
        }
        content_ += "\n";
        needs_comma_ = true;
    }

public:
    void add_field(const std::string& name, const std::string& value) override {
        handle_comma();
        content_ += get_indent() + "\"" + name + "\": \"" + value + "\"";
    }

    void add_field(const std::string& name, int value) override {
        handle_comma();
        content_ += get_indent() + "\"" + name + "\": " + std::to_string(value);
    }

    void add_field(const std::string& name, double value) override {
        handle_comma();
        content_ += get_indent() + "\"" + name + "\": " + std::to_string(value);
    }

    void add_block(const std::string& name) override {
        handle_comma();
        content_ += get_indent() + "\"" + name + "\": {";
        blocks_.push_back(name);
        indent_level_++;
        needs_comma_ = false;
    }

    void end_block() override {
        if (!blocks_.empty()) {
            indent_level_--;
            content_ += "\n" + get_indent() + "}";
            blocks_.pop_back();
            needs_comma_ = true;
        }
    }

    std::string build() override {
        while (!blocks_.empty()) {
            end_block();
        }
        return "{\n" + content_ + "\n}";
    }
};

// Базовый класс Vehicle
class Vehicle {
public:
    virtual ~Vehicle() = default;
    virtual void serialize(Serializer& serializer) const = 0;

    std::string name;
    std::string manufacturer;
    double weight;
    double power;
    int year;
};

// Автомобиль
class Car : public Vehicle {
public:
    int doors;
    int passenger_seats;
    std::string fuel_type;
    double engine_volume;

    void serialize(Serializer& serializer) const override {
        serializer.add_block("vehicle");
        serializer.add_field("type", "Car");
        serializer.add_field("name", name);
        serializer.add_field("manufacturer", manufacturer);
        serializer.add_field("weight", weight);
        serializer.add_field("power", power);
        serializer.add_field("year", year);

        serializer.add_block("car_specific");
        serializer.add_field("doors", doors);
        serializer.add_field("passenger_seats", passenger_seats);
        serializer.add_field("fuel_type", fuel_type);
        serializer.add_field("engine_volume", engine_volume);
        serializer.end_block();

        serializer.end_block();
    }
};

// Самолет
class Airplane : public Vehicle {
public:
    int wingspan;
    int max_altitude;
    int passenger_capacity;
    double max_speed;

    void serialize(Serializer& serializer) const override {
        serializer.add_block("vehicle");
        serializer.add_field("type", "Airplane");
        serializer.add_field("name", name);
        serializer.add_field("manufacturer", manufacturer);
        serializer.add_field("weight", weight);
        serializer.add_field("power", power);
        serializer.add_field("year", year);

        serializer.add_block("airplane_specific");
        serializer.add_field("wingspan", wingspan);
        serializer.add_field("max_altitude", max_altitude);
        serializer.add_field("passenger_capacity", passenger_capacity);
        serializer.add_field("max_speed", max_speed);
        serializer.end_block();

        serializer.end_block();
    }
};

// Корабль
class Ship : public Vehicle {
public:
    double length;
    double displacement;
    int crew_capacity;
    std::string propulsion_type;

    void serialize(Serializer& serializer) const override {
        serializer.add_block("vehicle");
        serializer.add_field("type", "Ship");
        serializer.add_field("name", name);
        serializer.add_field("manufacturer", manufacturer);
        serializer.add_field("weight", weight);
        serializer.add_field("power", power);
        serializer.add_field("year", year);

        serializer.add_block("ship_specific");
        serializer.add_field("length", length);
        serializer.add_field("displacement", displacement);
        serializer.add_field("crew_capacity", crew_capacity);
        serializer.add_field("propulsion_type", propulsion_type);
        serializer.end_block();

        serializer.end_block();
    }
};

// Функция сериализации
void serialize(const Vehicle& vehicle, Serializer& serializer) {
    vehicle.serialize(serializer);
}

// Фабрика для создания сериализаторов
std::unique_ptr<Serializer> create_serializer(const std::string& format) {
    if (format == "xml") {
        return std::make_unique<XmlSerializer>();
    }
    else if (format == "json") {
        return std::make_unique<JsonSerializer>();
    }
    throw std::invalid_argument("Unsupported format: " + format);
}

// Функция для преобразования строки в верхний регистр
std::string to_uppercase(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = std::toupper(c);
    }
    return result;
}

int main() {
    std::string format;

    // Спрашиваем у пользователя формат
    std::cout << "Choose format (json/xml): ";
    std::cin >> format;

    // Приводим к нижнему регистру для проверки
    for (char& c : format) {
        c = std::tolower(c);
    }

    if (format != "json" && format != "xml") {
        std::cout << "Invalid format. Using JSON by default.\n";
        format = "json";
    }

    try {
        auto serializer = create_serializer(format);

        // Создаем транспортные средства
        Car bmw;
        bmw.name = "BMW G30";
        bmw.manufacturer = "BMW";
        bmw.weight = 1600;
        bmw.power = 252;
        bmw.year = 2020;
        bmw.doors = 4;
        bmw.passenger_seats = 5;
        bmw.fuel_type = "petrol";
        bmw.engine_volume = 2.0;

        Airplane boeing;
        boeing.name = "Boeing 747-400";
        boeing.manufacturer = "Boeing";
        boeing.weight = 180000;
        boeing.power = 240000;
        boeing.year = 1988;
        boeing.wingspan = 64;
        boeing.max_altitude = 13700;
        boeing.passenger_capacity = 416;
        boeing.max_speed = 988;

        Ship queen_victoria;
        queen_victoria.name = "MS Queen Victoria";
        queen_victoria.manufacturer = "Fincantieri";
        queen_victoria.weight = 90000000;
        queen_victoria.power = 120000;
        queen_victoria.year = 2007;
        queen_victoria.length = 294;
        queen_victoria.displacement = 90000;
        queen_victoria.crew_capacity = 1000;
        queen_victoria.propulsion_type = "diesel-electric";

        // Сериализуем все транспортные средства
        std::vector<Vehicle*> vehicles = { &bmw, &boeing, &queen_victoria };

        std::cout << "\n=== Serialized vehicles in " << to_uppercase(format) << " format ===\n\n";

        for (auto vehicle : vehicles) {
            serialize(*vehicle, *serializer);
            std::cout << serializer->build() << "\n\n";
            // Сбрасываем сериализатор для следующего vehicle
            serializer = create_serializer(format);
        }

    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}


