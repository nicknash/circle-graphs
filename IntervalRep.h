#include <vector>
#include <stack>
#include <optional>

class Interval
{
public:
        const int Left;
        const int Right;
        const int Index;
};

class SimpleIntervalRep
{
public:
    const int End;
    const int Size;

    [[nodiscard]] std::optional<Interval> TryGetIntervalFromRightEndpoint(int maybeRightEndpoint) const
    {
       
    }

    
};

// An implementation of the output sensitive algorithm from 
// "An output sensitive algorithm for computing a maximum independent set of a circle graph", 2010, Inf. Process. Lett. 110(16) pp630-634 
class OutputSensitive
{
private:
    static void UpdateAt(std::stack<int>& pendingUpdates, std::vector<int>& MIS, int indexToUpdate, int newValue)
    {
        MIS[indexToUpdate] = newValue;
        pendingUpdates.push(indexToUpdate);        
    }
    
    static void Update(const SimpleIntervalRep& intervals, std::stack<int>& pendingUpdates, const Interval& interval, std::vector<int>& MIS, std::vector<int>& CMIS)
    {
        UpdateAt(pendingUpdates, MIS, interval.Left, 1 + CMIS[interval.Index]);
        while(!pendingUpdates.empty())
        {
            auto nextToUpdate = pendingUpdates.top();
            pendingUpdates.pop();
            auto leftNeighbour = nextToUpdate - 1;
            if(nextToUpdate > 0 && MIS[nextToUpdate] > MIS[leftNeighbour])
            {
                UpdateAt(pendingUpdates, MIS, leftNeighbour, MIS[nextToUpdate]);
            }
            auto maybeInterval = intervals.TryGetIntervalFromRightEndpoint(leftNeighbour);
            if(maybeInterval)
            {
                auto interval = maybeInterval.value();
                auto candidate = 1 + CMIS[interval.Index] + MIS[nextToUpdate];
                if(candidate > MIS[interval.Left])
                {
                    UpdateAt(pendingUpdates, MIS, interval.Left, candidate);
                }
            }
        }
    }

public:
    static void ComputeMIS(const SimpleIntervalRep& intervals)
    {
        std::vector<int> MIS(intervals.End, 0);
        std::vector<int> CMIS(intervals.Size, 0);
        std::stack<int> pendingUpdates;
        for(auto i = 0; i < intervals.End; ++i)
        {
            auto maybeInterval = intervals.TryGetIntervalFromRightEndpoint(i);
            if(maybeInterval)
            {
                auto interval = maybeInterval.value();
                CMIS[interval.Index] = MIS[interval.Left + 1];
                Update(intervals, pendingUpdates, interval, MIS, CMIS);
            }
        }
    }
};