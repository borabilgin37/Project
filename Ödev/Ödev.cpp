#include <iostream>
#include <vector>
#include <random>
#include <algorithm>
#include <numeric>
#include <map>

class Politician {
public:
    int age;
    int yearsInService;
    int rank; // 0 - Quaestor, 1 - Aedile, 2 - Praetor, 3 - Consul
    bool isConsulCandidate;
    int lastConsulElectionYear;

    Politician(int _age, int _rank) : age(_age), yearsInService(0), rank(_rank), isConsulCandidate(false), lastConsulElectionYear(-10) {}
};

class Simulation {
    std::vector<Politician> politicians;
    std::vector<int> availablePositions{ 20, 10, 8, 2 }; // Quaestors, Aediles, Praetors, Consuls
    std::default_random_engine generator{ std::random_device{}() };
    std::normal_distribution<double> candidateDist{ 15, 5 };
    int PSI;
    int year;

public:
    Simulation() : PSI(100), year(0) {
        initializePoliticians();
    }

    void run() {
        for (year = 1; year <= 200; ++year) {
            simulateYear();
        }
        reportResults();
    }

private:
    void initializePoliticians() {
        std::vector<int> minAges{ 30, 36, 39, 42 };
        for (int rank = 0; rank < availablePositions.size(); ++rank) {
            for (int i = 0; i < availablePositions[rank]; ++i) {
                politicians.push_back(Politician(minAges[rank], rank));
            }
        }
    }

    void simulateYear() {
        for (auto& p : politicians) {
            p.age++;
            p.yearsInService++;
        }
        politicians.erase(std::remove_if(politicians.begin(), politicians.end(), [this](const Politician& p) {
            return p.age > 80;
            }), politicians.end());

        int newCandidates = std::round(candidateDist(generator));
        for (int i = 0; i < newCandidates; ++i) {
            politicians.push_back(Politician(30, 0));
        }

        simulateElectionsAndPromotions();
        adjustPSI();
    }

    void simulateElectionsAndPromotions() {
        std::vector<int> minAges{ 30, 36, 39, 42 };
        std::vector<int> serviceYearsRequired{ 0, 2, 2, 2 };
        std::map<int, std::vector<Politician*>> eligibleCandidates;

        for (auto& p : politicians) {
            if (p.age >= minAges[p.rank] && p.yearsInService >= serviceYearsRequired[p.rank]) {
                eligibleCandidates[p.rank + 1].push_back(&p);
            }
        }

        for (int rank = 3; rank >= 0; --rank) {
            auto& candidates = eligibleCandidates[rank];
            int positionsToFill = availablePositions[rank] - std::count_if(politicians.begin(), politicians.end(), [rank](const Politician& p) { return p.rank == rank; });

            std::shuffle(candidates.begin(), candidates.end(), generator);

            for (int i = 0; i < positionsToFill && i < candidates.size(); ++i) {
                candidates[i]->rank = rank;
                if (rank == 3) {
                    candidates[i]->lastConsulElectionYear = year;
                }
            }
        }
    }

    void adjustPSI() {
        for (int rank = 0; rank < availablePositions.size(); ++rank) {
            int filledPositions = std::count_if(politicians.begin(), politicians.end(), [rank](const Politician& p) { return p.rank == rank; });
            int unfilledPositions = availablePositions[rank] - filledPositions;
            PSI -= unfilledPositions * 5;

            for (auto& p : politicians) {
                if (p.rank == 3 && (year - p.lastConsulElectionYear) < 10) {
                    PSI -= 10;
                }
            }
        }
    }

    void reportResults() {
        std::cout << "Final PSI: " << PSI << std::endl;

        // Report fill rates for each rank
        for (int rank = 0; rank < availablePositions.size(); ++rank) {
            int filledPositions = std::count_if(politicians.begin(), politicians.end(), [rank](const Politician& p) { return p.rank == rank; });
            double fillRate = 100.0 * filledPositions / availablePositions[rank];
            std::cout << "Fill Rate for Rank " << rank << ": " << fillRate << "%" << std::endl;
        }

        // Report age distribution for each rank
        std::map<int, std::vector<int>> ageDistributions;
        for (const auto& p : politicians) {
            ageDistributions[p.rank].push_back(p.age);
        }

        for (int rank = 0; rank < availablePositions.size(); ++rank) {
            if (ageDistributions.find(rank) != ageDistributions.end()) {
                const auto& ages = ageDistributions[rank];
                double averageAge = std::accumulate(ages.begin(), ages.end(), 0.0) / ages.size();
                auto minIt = std::min_element(ages.begin(), ages.end());
                auto maxIt = std::max_element(ages.begin(), ages.end());
                std::cout << "Age Distribution for Rank " << rank << ": Avg = " << averageAge
                    << ", Min = " << *minIt << ", Max = " << *maxIt << std::endl;
            }
            else {
                std::cout << "No politicians currently in Rank " << rank << std::endl;
            }
        }
    }
};

int main() {
    Simulation sim; // Create an instance of the Simulation class
    sim.run(); // Start the simulation

    return 0; // Indicate successful completion
}

