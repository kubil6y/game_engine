# Data Oriented Design

-   Data-oriented design is an optimization approach motivated by efficient usage of the CPU cache, largely populatrized by game developers.
-   The claim is that traditional OOP design principles result in poor data locality.
-   The approach is to focus on the data layout, seperating and sorting fields according to when they are needed, and primarily thinking about transformations of data.

-   Famous proponents include Mike Akton & Scott Meyers

## Recommended resources

Videos:

-   [Data locality by Bob Nystrom](http://gameprogrammingpatterns.com/data-locality.html)

-   [Scott Meyers: Cpu Caches and Why You Care](https://www.youtube.com/watch?v=WDIkqP4JbkE)

-   [CppCon 2014: Mike Acton "Data-Oriented Design and C++" ](https://www.youtube.com/watch?v=rX0ItVEVjHc)

Books:

-   [Data-oriented design: software engineering for limited resources and short schedules by Mr Richard Fabian](https://www.amazon.com/Data-oriented-design-engineering-resources-schedules/dp/1916478700#customerReviews)

## CPU Cache

-   A CPU cache is a **hardware cache** used by the central processing unit (CPU) to reduce the average cost to access data from the main memory

-   Cache is a smaller, faster memory, located closer to a processor core, which stores copies of the data from **frequently used** memory locations

-   Most processors have a hierarchy of multiple cache **levels**:
    -   L1 > L2 > L3 > L4
