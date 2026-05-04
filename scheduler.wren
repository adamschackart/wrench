/* -----------------------------------------------------------------------------
--- Copyright (c) 2012-2022 Adam Schackart / "AJ Hackman", all rights reserved.
--- Distributed under the BSD license v2 (opensource.org/licenses/BSD-3-Clause)
--------------------------------------------------------------------------------
--- Scheduling for complex, multi-step game entity behavior sequence scripting.
----------------------------------------------------------------------------- */

class Task {
    construct new(scheduler) {
        _scheduler = scheduler

        _timeElapsed = 0.0
        _timeRemaining = 0.0

        _waitElapsed = 0.0
        _waitRemaining = 0.0
    }

    static create(scheduler, timeRemaining, data, func) {
        var task = Task.new(scheduler)

        task.timeRemaining = timeRemaining
        task.data = data
        task.fiber = func

        return task
    }

    static create(scheduler, data, func) {
        var task = Task.new(scheduler)

        task.data = data
        task.fiber = func

        return task
    }

    static create(scheduler, func) {
        var task = Task.new(scheduler)

        task.fiber = func
        return task
    }

    scheduler { _scheduler }
    scheduler=(value) { _scheduler = value }

    timeElapsed { _timeElapsed }
    timeElapsed=(value) { _timeElapsed = value }

    timeRemaining { _timeRemaining }
    timeRemaining=(value) { _timeRemaining = value }

    waitElapsed { _waitElapsed }
    waitElapsed=(value) { _waitElapsed = value }

    waitRemaining { _waitRemaining }
    waitRemaining=(value) { _waitRemaining = value }

    isWaiting { _waitRemaining > 0.0 }

    fiber { _fiber }
    fiber=(value) {
        if (value is Fiber) {
            _fiber = value
        } else if (value is Fn) {
            _fiber = Fiber.new(value)
        } else if (value is Task) {
            _fiber = value.fiber
        } else {
            if (true) {
                /*
                 * Capture in upvalue and try to call.
                 */
                _fiber = Fiber.new { value.call() }
            } else {
                Fiber.abort("%(value)")
            }
        }
    }

    data { _data }
    data=(value) { _data = value }

    wait(seconds) {
        waitElapsed = 0.0
        waitRemaining = seconds

        Fiber.yield()
    }

    /* TODO: This will have to be a macro, as the condition must be re-evaluated
    ~* for each iteration of the loop. Leaving this to document a common pattern.

    waitUntil(condition) {
        while (!condition) {
            Fiber.yield()
        }
    }*/
}

class Scheduler {
    construct new() {
        _tasks = []
    }

    tasks { _tasks }
    tasks=(value) { _tasks = value }

    ! { tasks.isEmpty }

    update(dt) {
        /*
         * Iterate over a copy of the list in case elements are removed.
         */
        for (task in tasks.toList) {
            if (task.timeRemaining < 0.000001 && task.fiber.isDone) {
                tasks.remove(task)
            } else if (!task.isWaiting && !task.fiber.isDone) {
                task.fiber.call(task)
            }

            task.timeElapsed = task.timeElapsed + dt
            task.timeRemaining = task.timeRemaining - dt

            task.waitElapsed = task.waitElapsed + dt
            task.waitRemaining = task.waitRemaining - dt
        }

        return this
    }

    add(timeRemaining, data, task) {
        return add(Task.create(this, timeRemaining, data, task))
    }

    add(data, task) {
        return add(Task.create(this, data, task))
    }

    add(task) {
        if (task is Task) {
            return tasks.add(task)
        } else {
            return add(Task.create(this, task))
        }
    }
}

/* TODO: Improve this little test/example. Should print:
 *
 * A phase 1
 * A phase 2
 * B phase 1
 * A phase 3
 * B phase 2
 */
var main = Fn.new {
    import "time" for Timer
    var scheduler = Scheduler.new()

    var flag = false

    scheduler.add(0.4) { |task|
        System.print("A phase 1")
        task.wait(0.2)
        System.print("A phase 2")
        flag = true
        task.wait(task.data)
        System.print("A phase 3")
    }

    scheduler.add { |task|
        while (!flag) Fiber.yield()
        System.print("B phase 1")
        task.wait(0.7)
        System.print("B phase 2")
    }

    for (i in 0...60) {
        scheduler.update(1.0 / 60.0)
        Timer.sleepMS((1000.0 / 60.0).floor)
    }
}
