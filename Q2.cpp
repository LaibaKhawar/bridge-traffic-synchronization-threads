
#include <iostream>
#include <pthread.h>
#include <unistd.h>

using namespace std;

pthread_mutex_t bridgeMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t canCrossBridge = PTHREAD_COND_INITIALIZER;
pthread_cond_t canCrossBridgeForBus = PTHREAD_COND_INITIALIZER;

struct Vehicle {
    int direction;
    int type;  // 0 for car, 1 for bus as mentioned
};

void printWaitingVehicle(Vehicle* waitingVehicle) {
    cout << "Vehicle (Type: " << (waitingVehicle->type == 0 ? "Car" : "Bus")
         << ", Direction: " << waitingVehicle->direction << ") is waiting.\n";
}

void* vehicleThread(void* arg) {
    Vehicle* vehicle = static_cast<Vehicle*>(arg); //type casting

    pthread_mutex_lock(&bridgeMutex);

    static int carsOnBridge = 0; //initially set the number of cars and buses
    static int busesOnBridge = 0;

    // Check if two buses are on the bridge...as not allowed in the question
    while (vehicle->type == 1 && busesOnBridge == 1) {
        printWaitingVehicle(vehicle);
        pthread_cond_wait(&canCrossBridgeForBus, &bridgeMutex);
    }

    // Check if the bridge is full or if there is another bus waiting so that wait can be applied on the bus
    while ((vehicle->type == 0 && (carsOnBridge >= 2 || busesOnBridge > 0)) ||
           (vehicle->type == 1 && (busesOnBridge > 0 || carsOnBridge > 0))) {
        printWaitingVehicle(vehicle);
        if (vehicle->type == 1) {
            pthread_cond_wait(&canCrossBridgeForBus, &bridgeMutex);
        } else {
            pthread_cond_wait(&canCrossBridge, &bridgeMutex);
        }
    }

    if (vehicle->type == 1) {
        busesOnBridge++;
        // Signal to the waiting buses that it's their turn
        pthread_cond_broadcast(&canCrossBridgeForBus);
    } else {
        carsOnBridge++;
        // Signal to the waiting vehicles that it's their turn
        pthread_cond_broadcast(&canCrossBridge);
    }

    pthread_mutex_unlock(&bridgeMutex);

    // Crossing on the bridge
    cout << "Vehicle (Type: " << (vehicle->type == 0 ? "Car" : "Bus")
         << ", Direction: " << vehicle->direction << ") is crossing the bridge.\n";

    sleep(1); // Sleep for 1 second..so that vehicles cross

    pthread_mutex_lock(&bridgeMutex);

    if (vehicle->type == 1) {
        busesOnBridge--;

        // Signal to the waiting buses that it's their turn
        pthread_cond_signal(&canCrossBridgeForBus);
    } else {
        carsOnBridge--;

        // Signal to the waiting vehicles that it's their turn
        pthread_cond_broadcast(&canCrossBridge);
    }

    pthread_mutex_unlock(&bridgeMutex);

    // Simulate time spent off the bridge
    sleep(1); // Sleep for 1 second

    delete vehicle;

    pthread_exit(NULL);
}

int main() {
    srand(time(NULL));

    int maxCars, maxBuses;
    cout << "Enter the maximum number of cars: ";
    cin >> maxCars;
    cout << "Enter the maximum number of buses: ";
    cin >> maxBuses;

    int direction;
    cout << "Enter the direction for all vehicles (0 or 1): ";
    cin >> direction;

    const int numThreads = maxCars + maxBuses;  // Number of threads to be created
    pthread_t threads[numThreads];

    int carCount = 0, busCount = 0;

    for (int i = 0; i < numThreads; ++i) {
        int randomType = rand() % 2;  // 0 for car, 1 for bus

        if ((randomType == 0 && carCount < maxCars) || (randomType == 1 && busCount < maxBuses)) {
            Vehicle* vehicle = new Vehicle{direction, randomType};
            pthread_create(&threads[i], NULL, vehicleThread, vehicle);
            sleep(1); // Sleep for 1 second
            if (randomType == 0) {
                carCount++;
            } else {
                busCount++;
            }
        } else {
            i--; // Decrement i to try again with a different random type
        }
    }

    sleep(60); // Sleep for 60 seconds

    for (int i = 0; i < numThreads; ++i) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

