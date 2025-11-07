#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <stdexcept>
#include <cctype>

class Serializer {
public:
    virtual ~Serializer() = default;

    virtual void addField(const std::string& fieldName, const std::string& fieldValue) = 0;
    virtual void addField(const std::string& fieldName, int fieldValue) = 0;
    virtual void addField(const std::string& fieldName, double fieldValue) = 0;

    virtual void addBlock(const std::string& blockName) = 0;
    virtual void endBlock() = 0;

    virtual std::string build() = 0;
};

class XmlSerializer : public Serializer {
private:
    std::vector<std::string> blockStack;
    std::string outputContent;
    int currentIndentLevel = 0;

    std::string getCurrentIndent() const {
        return std::string(currentIndentLevel * 2, ' ');
    }

public:
    void addField(const std::string& fieldName, const std::string& fieldValue) override {
        outputContent += getCurrentIndent() + "<" + fieldName + ">" + fieldValue + "</" + fieldName + ">\n";
    }

    void addField(const std::string& fieldName, int fieldValue) override {
        outputContent += getCurrentIndent() + "<" + fieldName + ">" + std::to_string(fieldValue) + "</" + fieldName + ">\n";
    }

    void addField(const std::string& fieldName, double fieldValue) override {
        outputContent += getCurrentIndent() + "<" + fieldName + ">" + std::to_string(fieldValue) + "</" + fieldName + ">\n";
    }

    void addBlock(const std::string& blockName) override {
        outputContent += getCurrentIndent() + "<" + blockName + ">\n";
        blockStack.push_back(blockName);
        currentIndentLevel++;
    }

    void endBlock() override {
        if (!blockStack.empty()) {
            currentIndentLevel--;
            std::string lastBlockName = blockStack.back();
            blockStack.pop_back();
            outputContent += getCurrentIndent() + "</" + lastBlockName + ">\n";
        }
    }

    std::string build() override {
        while (!blockStack.empty()) {
            endBlock();
        }
        return outputContent;
    }
};

class JsonSerializer : public Serializer {
private:
    std::vector<std::string> blockStack;
    std::string outputContent;
    bool isCommaNeeded = false;
    int currentIndentLevel = 0;

    std::string getCurrentIndent() const {
        return std::string(currentIndentLevel * 2, ' ');
    }

    void handleCommaIfNeeded() {
        if (isCommaNeeded) {
            outputContent += ",";
        }
        outputContent += "\n";
        isCommaNeeded = true;
    }

public:
    void addField(const std::string& fieldName, const std::string& fieldValue) override {
        handleCommaIfNeeded();
        outputContent += getCurrentIndent() + "\"" + fieldName + "\": \"" + fieldValue + "\"";
    }

    void addField(const std::string& fieldName, int fieldValue) override {
        handleCommaIfNeeded();
        outputContent += getCurrentIndent() + "\"" + fieldName + "\": " + std::to_string(fieldValue);
    }

    void addField(const std::string& fieldName, double fieldValue) override {
        handleCommaIfNeeded();
        outputContent += getCurrentIndent() + "\"" + fieldName + "\": " + std::to_string(fieldValue);
    }

    void addBlock(const std::string& blockName) override {
        handleCommaIfNeeded();
        outputContent += getCurrentIndent() + "\"" + blockName + "\": {";
        blockStack.push_back(blockName);
        currentIndentLevel++;
        isCommaNeeded = false;
    }

    void endBlock() override {
        if (!blockStack.empty()) {
            currentIndentLevel--;
            outputContent += "\n" + getCurrentIndent() + "}";
            blockStack.pop_back();
            isCommaNeeded = true;
        }
    }

    std::string build() override {
        while (!blockStack.empty()) {
            endBlock();
        }
        return "{\n" + outputContent + "\n}";
    }
};

class Vehicle {
public:
    virtual ~Vehicle() = default;
    virtual void serialize(Serializer& serializer) const = 0;

    std::string modelName;
    std::string manufacturerName;
    double vehicleWeight;
    double enginePower;
    int productionYear;
};

class Car : public Vehicle {
public:
    int doorCount;
    int passengerSeatCount;
    std::string fuelType;
    double engineVolume;

    void serialize(Serializer& serializer) const override {
        serializer.addBlock("vehicle");
        serializer.addField("type", "Car");
        serializer.addField("name", modelName);
        serializer.addField("manufacturer", manufacturerName);
        serializer.addField("weight", vehicleWeight);
        serializer.addField("power", enginePower);
        serializer.addField("year", productionYear);

        serializer.addBlock("carSpecific");
        serializer.addField("doors", doorCount);
        serializer.addField("passengerSeats", passengerSeatCount);
        serializer.addField("fuelType", fuelType);
        serializer.addField("engineVolume", engineVolume);
        serializer.endBlock();

        serializer.endBlock();
    }
};

class Airplane : public Vehicle {
public:
    int wingSpan;
    int maxAltitude;
    int maxPassengerCapacity;
    double maxSpeed;

    void serialize(Serializer& serializer) const override {
        serializer.addBlock("vehicle");
        serializer.addField("type", "Airplane");
        serializer.addField("name", modelName);
        serializer.addField("manufacturer", manufacturerName);
        serializer.addField("weight", vehicleWeight);
        serializer.addField("power", enginePower);
        serializer.addField("year", productionYear);

        serializer.addBlock("airplaneSpecific");
        serializer.addField("wingspan", wingSpan);
        serializer.addField("maxAltitude", maxAltitude);
        serializer.addField("passengerCapacity", maxPassengerCapacity);
        serializer.addField("maxSpeed", maxSpeed);
        serializer.endBlock();

        serializer.endBlock();
    }
};

class Ship : public Vehicle {
public:
    double shipLength;
    double shipDisplacement;
    int crewCapacity;
    std::string propulsionType;

    void serialize(Serializer& serializer) const override {
        serializer.addBlock("vehicle");
        serializer.addField("type", "Ship");
        serializer.addField("name", modelName);
        serializer.addField("manufacturer", manufacturerName);
        serializer.addField("weight", vehicleWeight);
        serializer.addField("power", enginePower);
        serializer.addField("year", productionYear);

        serializer.addBlock("shipSpecific");
        serializer.addField("length", shipLength);
        serializer.addField("displacement", shipDisplacement);
        serializer.addField("crewCapacity", crewCapacity);
        serializer.addField("propulsionType", propulsionType);
        serializer.endBlock();

        serializer.endBlock();
    }
};

void serializeVehicle(const Vehicle& vehicle, Serializer& serializer) {
    vehicle.serialize(serializer);
}

std::unique_ptr<Serializer> createSerializer(const std::string& outputFormat) {
    if (outputFormat == "xml") {
        return std::make_unique<XmlSerializer>();
    }
    else if (outputFormat == "json") {
        return std::make_unique<JsonSerializer>();
    }
    throw std::invalid_argument("Unsupported format: " + outputFormat);
}

std::string toUpperCase(const std::string& inputString) {
    std::string resultString = inputString;
    for (char& currentChar : resultString) {
        currentChar = std::toupper(currentChar);
    }
    return resultString;
}

int main() {
    std::string selectedFormat;

    std::cout << "Choose format (json/xml): ";
    std::cin >> selectedFormat;

    for (char& currentChar : selectedFormat) {
        currentChar = std::tolower(currentChar);
    }

    if (selectedFormat != "json" && selectedFormat != "xml") {
        std::cout << "Invalid format. Using JSON by default.\n";
        selectedFormat = "json";
    }

    try {
        auto formatSerializer = createSerializer(selectedFormat);

        Car bmwCar;
        bmwCar.modelName = "BMW G30";
        bmwCar.manufacturerName = "BMW";
        bmwCar.vehicleWeight = 1600;
        bmwCar.enginePower = 252;
        bmwCar.productionYear = 2020;
        bmwCar.doorCount = 4;
        bmwCar.passengerSeatCount = 5;
        bmwCar.fuelType = "petrol";
        bmwCar.engineVolume = 2.0;

        Airplane boeingPlane;
        boeingPlane.modelName = "Boeing 747-400";
        boeingPlane.manufacturerName = "Boeing";
        boeingPlane.vehicleWeight = 180000;
        boeingPlane.enginePower = 240000;
        boeingPlane.productionYear = 1988;
        boeingPlane.wingSpan = 64;
        boeingPlane.maxAltitude = 13700;
        boeingPlane.maxPassengerCapacity = 416;
        boeingPlane.maxSpeed = 988;

        Ship victoriaShip;
        victoriaShip.modelName = "MS Queen Victoria";
        victoriaShip.manufacturerName = "Fincantieri";
        victoriaShip.vehicleWeight = 90000000;
        victoriaShip.enginePower = 120000;
        victoriaShip.productionYear = 2007;
        victoriaShip.shipLength = 294;
        victoriaShip.shipDisplacement = 90000;
        victoriaShip.crewCapacity = 1000;
        victoriaShip.propulsionType = "diesel-electric";

        std::vector<Vehicle*> vehicleList = { &bmwCar, &boeingPlane, &victoriaShip };

        std::cout << "\n=== Serialized vehicles in " << toUpperCase(selectedFormat) << " format ===\n\n";

        for (auto currentVehicle : vehicleList) {
            serializeVehicle(*currentVehicle, *formatSerializer);
            std::cout << formatSerializer->build() << "\n\n";
            formatSerializer = createSerializer(selectedFormat);
        }

    }
    catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << std::endl;
        return 1;
    }

    return 0;
}
