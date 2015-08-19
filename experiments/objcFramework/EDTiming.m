// Licensed to the Apache Software Foundation (ASF) under one
// or more contributor license agreements.  See the NOTICE file
// distributed with this work for additional information
// regarding copyright ownership.  The ASF licenses this file
// to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance
// with the License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing,
// software distributed under the License is distributed on an
// "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied.  See the License for the
// specific language governing permissions and limitations
// under the License.

// This file comes from the portion of the UX Write editor that
// works on both Apple platforms (that is, it can run on either
// OS X or iOS). It's in the repository for illustrative purposes
// only, to assist with the creation of the framework for the
// Corinthia editor UI. The code does not compile independently in
// its present form.

#import "EDTiming.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          EDTimingEntry                                         //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDTimingEntry

- (EDTimingEntry *)initWithName:(NSString *)name
                         time:(NSTimeInterval)time
                          min:(NSTimeInterval)min
                          max:(NSTimeInterval)max
{
    if (!(self = [super init]))
        return nil;
    _name = [name copy];
    _time = time;
    _min = min;
    _max = max;
    return self;
}

- (EDTimingEntry *)initWithName:(NSString *)name time:(NSTimeInterval)time
{
    return [self initWithName: name time: time min: time max: time];
}

- (NSString *)description
{
    return [NSString stringWithFormat: @"%@: %dms", _name, (int)(_time*1000)];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                          EDTimingInfo                                          //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@interface EDTimingInfo()

@property (strong) NSDate *lastTime;

@end

@implementation EDTimingInfo
{
    NSMutableArray *_entries;
}

- (EDTimingInfo *)initWithEntries:(NSMutableArray *)entries
{
    if (!(self = [super init]))
        return nil;
    _entries = entries;
    for (EDTimingEntry *entry in _entries)
        _total += entry.time;
    return self;
}

- (EDTimingInfo *)init
{
    return [self initWithEntries: [NSMutableArray arrayWithCapacity: 0]];
}

- (void)start
{
    [_entries removeAllObjects];
    self.lastTime = [NSDate date];
}

- (void)addEntry:(NSString *)name
{
    if (self.lastTime == nil)
        [self start];

    NSDate *now = [NSDate date];
    NSTimeInterval interval = [now timeIntervalSinceDate: self.lastTime];
    self.lastTime = now;
    [_entries addObject: [[EDTimingEntry alloc] initWithName: name time: interval]];
    _total += interval;
}

- (NSString *)description
{
    NSMutableString *result = [NSMutableString stringWithCapacity: 0];
    for (EDTimingEntry *entry in _entries)
        [result appendFormat: @"%@\n", entry];
    [result appendFormat: @"Total: %dms", (int)(_total*1000)];
    return result;
}

- (NSString *)descriptionWithTitle:(NSString *)title
{
    NSMutableString *result = [NSMutableString stringWithCapacity: 0];
    [result appendFormat: @"%@\n",title];
    for (EDTimingEntry *entry in _entries)
        [result appendFormat: @"    %@\n", entry];
    [result appendFormat: @"    Total: %dms", (int)(_total*1000)];
    return result;
}

- (NSString *)detailedDescription
{
    // Use UTF8 strings here, since field width specifiers don't seem to work with %@
    NSMutableString *result = [NSMutableString stringWithCapacity: 0];
    [result appendFormat: @"%-40s %-8s %-8s %-8s\n", "Stage", "Avg", "Min", "Max"];
    [result appendFormat: @"%-40s %-8s %-8s %-8s\n", "-----", "---", "---", "---"];
    for (EDTimingEntry *entry in _entries) {
        [result appendFormat: @"%-40s %-8d %-8d %-8d\n",
         entry.name.UTF8String,
         (int)(entry.time*1000),
         (int)(entry.min*1000),
         (int)(entry.max*1000)];
    }
    [result appendFormat: @"Total: %dms", (int)(_total*1000)];
    return result;
}

- (BOOL)hasSameEntryNamesAs:(EDTimingInfo *)other
{
    if (self.entries.count != other.entries.count)
        return NO;
    for (NSUInteger i = 0; i < self.entries.count; i++) {
        EDTimingEntry *selfEntry = [self.entries objectAtIndex: i];
        EDTimingEntry *otherEntry = [other.entries objectAtIndex: i];
        if (![selfEntry.name isEqualToString: otherEntry.name])
            return NO;
    }
    return YES;
}

+ (EDTimingInfo *)computeAverages:(NSArray *)timings
{
    if (timings.count == 0)
        return nil;
    EDTimingInfo *first = [timings objectAtIndex: 0];

    for (EDTimingInfo *current in timings) {
        if (![current hasSameEntryNamesAs: first])
            return nil;
    }

    NSUInteger numEntries = first.entries.count;

    NSMutableArray *minArray = [NSMutableArray arrayWithCapacity: 0];
    NSMutableArray *maxArray = [NSMutableArray arrayWithCapacity: 0];
    NSMutableArray *totalArray = [NSMutableArray arrayWithCapacity: 0];
    for (NSUInteger i = 0; i < numEntries; i++) {
        [minArray addObject: [NSNumber numberWithDouble: 0]];
        [maxArray addObject: [NSNumber numberWithDouble: 0]];
        [totalArray addObject: [NSNumber numberWithDouble: 0]];
    }

    for (EDTimingInfo *current in timings) {
        for (NSUInteger i = 0; i < current.entries.count; i++) {
            EDTimingEntry *entry = [current.entries objectAtIndex: i];

            NSNumber *minVal = [minArray objectAtIndex: i];
            if ((current == first) || (minVal.doubleValue > entry.min)) {
                minVal = [NSNumber numberWithDouble: entry.min];
                [minArray replaceObjectAtIndex: i withObject: minVal];
            }

            NSNumber *maxVal = [maxArray objectAtIndex: i];
            if ((current == first) || (maxVal.doubleValue < entry.max)) {
                maxVal = [NSNumber numberWithDouble: entry.max];
                [maxArray replaceObjectAtIndex: i withObject: maxVal];
            }

            NSNumber *totalVal = [totalArray objectAtIndex: i];
            totalVal = [NSNumber numberWithDouble: totalVal.doubleValue + entry.time];
            [totalArray replaceObjectAtIndex: i withObject: totalVal];
        }
    }

    NSMutableArray *results = [NSMutableArray arrayWithCapacity: 0];
    for (NSUInteger i = 0; i < first.entries.count; i++) {
        EDTimingEntry *entry = [first.entries objectAtIndex: i];
        NSNumber *minVal = [minArray objectAtIndex: i];
        NSNumber *maxVal = [maxArray objectAtIndex: i];
        NSNumber *totalVal = [totalArray objectAtIndex: i];
        EDTimingEntry *combined = [[EDTimingEntry alloc] initWithName: entry.name
                                                             time: totalVal.doubleValue/timings.count
                                                              min: minVal.doubleValue
                                                              max: maxVal.doubleValue];
        [results addObject: combined];
    }

    return [[EDTimingInfo alloc] initWithEntries: results];
}

@end

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                //
//                                         EDTimingRecords                                        //
//                                                                                                //
////////////////////////////////////////////////////////////////////////////////////////////////////

@implementation EDTimingRecords
{
    NSTimeInterval *_values;
}

- (EDTimingRecords *)initWithMax:(NSUInteger)max
{
    if (!(self = [super init]))
        return nil;
    _max = max;
    _count = 0;
    _values = (NSTimeInterval *)malloc(_max*sizeof(NSTimeInterval));
    return self;
}

- (void)dealloc
{
    free(_values);
}

- (void)add:(NSTimeInterval)interval
{
    if (_count == _max) {
        for (NSUInteger i = 0; i < _max-1; i++)
            _values[i] = _values[i+1];
    }
    else {
        _count++;
    }
    _values[_count-1] = interval;
}

- (NSTimeInterval)average
{
    if (_count == 0)
        return 0;

    NSTimeInterval total = 0;
    for (NSUInteger i = 0; i < _count; i++)
        total += _values[i];
    return total/_count;
}

@end
