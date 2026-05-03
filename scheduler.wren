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
            Fiber.abort("%(value)")
        }
    }

    data { _data }
    data=(value) { _data = value }

    wait(seconds) {
        waitElapsed = 0.0
        waitRemaining = seconds

        Fiber.yield()
    }
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
