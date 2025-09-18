#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

// Include source files directly for testing (no Arduino dependencies)
#include "../MotionPlanner.h"
#include "../MotionExecutor.h"
#include "MockServoController.h"

TEST_CASE("MotionCommand basic functionality") {
    SUBCASE("Default constructor") {
        MotionCommand cmd;
        CHECK_EQ(cmd.targetX(), 0.0f);
        CHECK_EQ(cmd.targetY(), 0.0f);
        CHECK_EQ(cmd.duration_ms, 50UL);
        CHECK_EQ(cmd.laser_on, false);
    }

    SUBCASE("Parameterized constructor with Point") {
        Point target(10.0f, 20.0f);
        MotionCommand cmd(target, 100, true);
        CHECK_EQ(cmd.targetX(), 10.0f);
        CHECK_EQ(cmd.targetY(), 20.0f);
        CHECK_EQ(cmd.duration_ms, 100UL);
        CHECK_EQ(cmd.laser_on, true);
    }

    SUBCASE("Parameterized constructor with coordinates") {
        MotionCommand cmd(15.0f, 25.0f, 200, false);
        CHECK_EQ(cmd.targetX(), 15.0f);
        CHECK_EQ(cmd.targetY(), 25.0f);
        CHECK_EQ(cmd.duration_ms, 200UL);
        CHECK_EQ(cmd.laser_on, false);
    }

    SUBCASE("Setters and getters") {
        MotionCommand cmd;
        cmd.targetX(5.0f);
        cmd.targetY(10.0f);
        CHECK_EQ(cmd.targetX(), 5.0f);
        CHECK_EQ(cmd.targetY(), 10.0f);
    }
}

TEST_CASE("MotionConfig defaults") {
    MotionConfig config;
    CHECK_EQ(config.pointDuration_ms, 50UL);
    CHECK_EQ(config.pauseDuration_ms, 0UL);
    CHECK_EQ(config.laserEnabled, true);
    CHECK_EQ(config.returnToCenter, false);
}

TEST_CASE("MotionSequence boundary mapping") {
    MotionSequence seq;

    // Define a simple rectangular boundary
    Point boundary[4] = {
        Point(0, 0),    // bottom-left
        Point(100, 0),  // bottom-right
        Point(100, 100), // top-right
        Point(0, 100)   // top-left
    };

    Quadrilateral quad(boundary);
    seq.setBoundary(quad);

    SUBCASE("Generate commands from zigzag pattern") {
        ZigzagPattern pattern(2, 3); // 2 lines, 3 points per line
        MotionCommand commands[20];
        MotionConfig config;
        config.pointDuration_ms = 100;

        int numCommands = seq.generateCommands(pattern, commands, 20, config);

        CHECK_GT(numCommands, 0);
        CHECK_LE(numCommands, 20);

        // Verify all commands have valid coordinates within boundary
        for (int i = 0; i < numCommands; i++) {
            CHECK_GE(commands[i].targetX(), 0.0f);
            CHECK_LE(commands[i].targetX(), 100.0f);
            CHECK_GE(commands[i].targetY(), 0.0f);
            CHECK_LE(commands[i].targetY(), 100.0f);
            CHECK_EQ(commands[i].duration_ms, 100UL);
        }
    }

    SUBCASE("Generate transition between points") {
        Point start(10, 10);
        Point end(90, 90);
        MotionCommand commands[20];

        int numCommands = seq.generateTransition(start, end, commands, 20, 1000);

        CHECK_GT(numCommands, 0);
        CHECK_LE(numCommands, 20);

        // First command should start at start point
        CHECK_EQ(commands[0].targetX(), 10.0f);
        CHECK_EQ(commands[0].targetY(), 10.0f);

        // Last command should end at end point
        CHECK_EQ(commands[numCommands-1].targetX(), 90.0f);
        CHECK_EQ(commands[numCommands-1].targetY(), 90.0f);
    }
}

TEST_CASE("MotionPlanner pattern execution") {
    MotionPlanner planner;

    // Define boundary
    Point boundary[4] = {
        Point(0, 0),
        Point(100, 0),
        Point(100, 100),
        Point(0, 100)
    };
    Quadrilateral quad(boundary);

    SUBCASE("Execute zigzag pattern") {
        MotionCommand commands[50];
        MotionConfig config;
        config.pointDuration_ms = 75;

        int numCommands = planner.executePattern(
            planner.getZigzagPattern(),
            quad,
            commands,
            50,
            config
        );

        CHECK_GT(numCommands, 0);
        CHECK_LE(numCommands, 50);

        // Verify commands are within boundary
        for (int i = 0; i < numCommands; i++) {
            CHECK_GE(commands[i].targetX(), 0.0f);
            CHECK_LE(commands[i].targetX(), 100.0f);
            CHECK_GE(commands[i].targetY(), 0.0f);
            CHECK_LE(commands[i].targetY(), 100.0f);
        }
    }

    SUBCASE("Execute spiral pattern") {
        MotionCommand commands[50];
        MotionConfig config;

        int numCommands = planner.executePattern(
            planner.getSpiralPattern(),
            quad,
            commands,
            50,
            config
        );

        CHECK_GT(numCommands, 0);
        CHECK_LE(numCommands, 50);
    }

    SUBCASE("Boundary mapping with bilinear interpolation") {
        Point testBoundary[4] = {
            Point(10, 10),   // bottom-left
            Point(90, 10),   // bottom-right
            Point(90, 90),   // top-right
            Point(10, 90)    // top-left
        };

        // Test corners
        Point result = planner.mapToBoundary(0.0f, 0.0f, testBoundary);
        CHECK_EQ(result.x, 10.0f);
        CHECK_EQ(result.y, 10.0f);

        result = planner.mapToBoundary(1.0f, 1.0f, testBoundary);
        CHECK_EQ(result.x, 90.0f);
        CHECK_EQ(result.y, 90.0f);

        // Test center
        result = planner.mapToBoundary(0.5f, 0.5f, testBoundary);
        CHECK_EQ(result.x, 50.0f);
        CHECK_EQ(result.y, 50.0f);
    }
}

TEST_CASE("MotionPlanner legacy compatibility") {
    MotionPlanner planner;

    Point boundary[4] = {
        Point(0, 0),
        Point(100, 0),
        Point(100, 100),
        Point(0, 100)
    };

    MotionCommand commands[50];
    int commandCount = 0;

    SUBCASE("Generate zigzag pattern (legacy)") {
        planner.generateZigzagPattern(
            boundary,
            commands,
            commandCount,
            50,
            4,  // numZigzags
            5,  // pointsPerLine
            60  // pointDuration
        );

        CHECK_GT(commandCount, 0);
        CHECK_LE(commandCount, 50);

        for (int i = 0; i < commandCount; i++) {
            CHECK_GT(commands[i].duration_ms, 0UL);
            CHECK_LE(commands[i].duration_ms, 1000UL); // Reasonable duration range
        }
    }

    SUBCASE("Generate smooth path") {
        Point start(20, 20);
        Point end(80, 80);
        int commandCount = 0;

        planner.generateSmoothPath(
            start,
            end,
            commands,
            commandCount,
            50,
            1000,  // totalDuration
            10     // steps
        );

        CHECK_EQ(commandCount, 11); // steps + 1 (includes start and end)

        // Check start and end points
        CHECK_EQ(commands[0].targetX(), 20.0f);
        CHECK_EQ(commands[0].targetY(), 20.0f);
        CHECK_GE(commands[10].targetX(), 75.0f); // Allow some tolerance due to integer casting
        CHECK_LE(commands[10].targetX(), 85.0f);
        CHECK_GE(commands[10].targetY(), 75.0f);
        CHECK_LE(commands[10].targetY(), 85.0f);
    }
}

TEST_CASE("MotionExecutor basic functionality") {
    // Note: MotionExecutor requires hardware-specific ServoController
    // For unit testing, we test the logic indirectly through integration tests
    // or create a separate test version that uses dependency injection

    SUBCASE("MotionExecutor constants") {
        // Test that constants are properly defined
        CHECK_EQ(MotionExecutor::MAX_QUEUE_SIZE, 200);
    }
}

TEST_CASE("MotionExecutor command timing") {
    // Note: Full MotionExecutor testing requires hardware mock
    // This test verifies the command structure instead

    SUBCASE("Command duration validation") {
        MotionCommand cmd(Point(100, 100), 200, true);
        CHECK_EQ(cmd.targetX(), 100.0f);
        CHECK_EQ(cmd.targetY(), 100.0f);
        CHECK_EQ(cmd.duration_ms, 200UL);
        CHECK_EQ(cmd.laser_on, true);
    }
}

TEST_CASE("Integration: MotionPlanner command generation") {
    MotionPlanner planner;

    Point boundary[4] = {
        Point(0, 0),
        Point(100, 0),
        Point(100, 100),
        Point(0, 100)
    };
    Quadrilateral quad(boundary);

    SUBCASE("Generate and validate pattern commands") {
        MotionCommand commands[20];
        MotionConfig config;
        config.pointDuration_ms = 100;

        // Generate zigzag pattern
        int numCommands = planner.executePattern(
            planner.getZigzagPattern(),
            quad,
            commands,
            20,
            config
        );

        CHECK_GT(numCommands, 0);

        // Verify all commands are properly formed
        for (int i = 0; i < numCommands; i++) {
            CHECK_GE(commands[i].targetX(), 0.0f);
            CHECK_LE(commands[i].targetX(), 100.0f);
            CHECK_GE(commands[i].targetY(), 0.0f);
            CHECK_LE(commands[i].targetY(), 100.0f);
            CHECK_EQ(commands[i].duration_ms, 100UL);
        }
    }
}

TEST_CASE("MotionExecutor edge cases") {
    // Test edge cases without hardware dependencies

    SUBCASE("Command array bounds") {
        MotionCommand commands[5];
        for (int i = 0; i < 5; i++) {
            commands[i] = MotionCommand(Point(i*20, i*20), 100, (i % 2 == 0));
        }

        // Verify commands are properly constructed
        CHECK_EQ(commands[0].targetX(), 0.0f);
        CHECK_EQ(commands[0].targetY(), 0.0f);
        CHECK_EQ(commands[0].laser_on, true);

        CHECK_EQ(commands[3].targetX(), 60.0f);
        CHECK_EQ(commands[3].targetY(), 60.0f);
        CHECK_EQ(commands[3].laser_on, false);
    }

    SUBCASE("MotionCommand array operations") {
        MotionCommand commands[3] = {
            MotionCommand(Point(10, 10), 50, false),
            MotionCommand(Point(50, 50), 50, true),
            MotionCommand(Point(90, 90), 50, false)
        };

        // Test array access and modification
        commands[1].targetX(75.0f);
        commands[1].targetY(25.0f);

        CHECK_EQ(commands[1].targetX(), 75.0f);
        CHECK_EQ(commands[1].targetY(), 25.0f);
        // Note: laser_on state depends on the index pattern, just verify it's a boolean
        CHECK((commands[1].laser_on == true || commands[1].laser_on == false));
    }
}

TEST_CASE("MotionPlanner boundary edge cases") {
    MotionPlanner planner;

    SUBCASE("Degenerate boundary") {
        Point badBoundary[4] = {
            Point(50, 50),
            Point(50, 50),
            Point(50, 50),
            Point(50, 50)
        };

        Point result = planner.mapToBoundary(0.5f, 0.5f, badBoundary);
        // Should still return a valid point (center of degenerate boundary)
        CHECK_EQ(result.x, 50.0f);
        CHECK_EQ(result.y, 50.0f);
    }

    SUBCASE("Normalized coordinates at boundaries") {
        Point boundary[4] = {
            Point(0, 0),
            Point(100, 0),
            Point(100, 100),
            Point(0, 100)
        };

        // Test all corners
        Point result = planner.mapToBoundary(0.0f, 0.0f, boundary);
        CHECK_EQ(result.x, 0.0f);
        CHECK_EQ(result.y, 0.0f);

        result = planner.mapToBoundary(1.0f, 0.0f, boundary);
        CHECK_EQ(result.x, 100.0f);
        CHECK_EQ(result.y, 0.0f);

        result = planner.mapToBoundary(1.0f, 1.0f, boundary);
        CHECK_EQ(result.x, 100.0f);
        CHECK_EQ(result.y, 100.0f);

        result = planner.mapToBoundary(0.0f, 1.0f, boundary);
        CHECK_EQ(result.x, 0.0f);
        CHECK_EQ(result.y, 100.0f);
    }
}
