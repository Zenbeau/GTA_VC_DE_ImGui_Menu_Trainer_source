#pragma once
#include <cstdint>

extern "C"
{
    extern uintptr_t moduleBase;
    // Godmode
    extern bool Godmode_flag;
    extern uintptr_t JumpBack0;
    //extern uintptr_t JumpBack6;
    //extern uintptr_t JumpBack7;
    //extern uintptr_t conditionaljumpD;
    // One Hit Kill
    extern bool ohk_flag;
    extern uintptr_t JumpBack1;
    extern float dmgVal;
    // Wanted Level
    extern bool wanted_flag;
    extern uintptr_t JumpBack2;
    // Free Mouse Cursor
    extern bool show;
    extern uintptr_t JumpBack3;
    // Infinite Ammo
    extern bool ammo_flag;
    extern uintptr_t JumpBack4;
    extern uintptr_t conditionaljumpA;
    extern uintptr_t conditionaljumpB;
    // for calling Give Weapon from Main Thread
    extern uintptr_t JumpBack9;
    // Freeze Mission Timer
    extern bool timer1_flag;
    extern uintptr_t JumpBack5;
    extern uintptr_t conditionaljumpC;
    // get Waypoints
    //extern bool waypoints_flag;
    extern uintptr_t waypointBase;
    extern uintptr_t JumpBack8;
    // Can't fall off bike
    extern bool toggle10;
    extern uintptr_t JumpBack10;
    extern uintptr_t conditionaljumpF;
    // Fly
    //extern bool toggle1;
    //extern uintptr_t JumpBack1;
  
    void Repair_Car();
    void Shutdown(); // declare prototype here because Shutdown function is located under Endscene one

    void Godmode(); // Declare the Godmode ASM function
    void Godmode_vehicle();
    void Godmode_vehicle2();
    void OneHitKill();
    void wanted1();
    void FreeCursor();
    void InfiniteAmmo();
    void MissionTimer();
    void Waypoints();
    void MainThread();
    void OrderStuff();
    void AlwaysOnBike();
    
    //void Fly();
}







