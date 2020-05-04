#!/bin/bash
$* &
pid=$!
sudo insmod pf_probe_B.ko pid_param=$pid
wait $pid
sudo rmmod pf_probe_B
dmesg | tail -50
