# echo "hello world" > /dev/faulty
[   38.479672] Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000
[   38.480141] Mem abort info:
[   38.480270]   ESR = 0x96000044
[   38.480422]   EC = 0x25: DABT (current EL), IL = 32 bits
[   38.480632]   SET = 0, FnV = 0
[   38.484876]   EA = 0, S1PTW = 0
[   38.485044]   FSC = 0x04: level 0 translation fault
[   38.485263] Data abort info:
[   38.485393]   ISV = 0, ISS = 0x00000044
[   38.485559]   CM = 0, WnR = 1
[   38.485751] user pgtable: 4k pages, 48-bit VAs, pgdp=0000000043bea000
[   38.486035] [0000000000000000] pgd=0000000000000000, p4d=0000000000000000
[   38.486408] Internal error: Oops: 96000044 [#1] PREEMPT SMP
[   38.486674] Modules linked in: faulty(O) hello(O) scull(O) ipv6
[   38.487078] CPU: 0 PID: 198 Comm: sh Tainted: G           O      5.15.15 #1
[   38.487325] Hardware name: linux,dummy-virt (DT)
[   38.487578] pstate: 80000005 (Nzcv daif -PAN -UAO -TCO -DIT -SSBS BTYPE=--)
[   38.487796] pc : faulty_write+0x14/0x20 [faulty]
[   38.488174] lr : vfs_write+0xf0/0x290
[   38.488306] sp : ffff8000102a3d80
[   38.488410] x29: ffff8000102a3d80 x28: ffff235e0275d280 x27: 0000000000000000
[   38.488678] x26: 0000000000000000 x25: 0000000000000000 x24: 0000000000000000
[   38.488970] x23: 0000000000000000 x22: ffff8000102a3dc0 x21: 0000aaab0a174a20
[   38.489205] x20: ffff235e03a2c100 x19: 000000000000000c x18: 0000000000000000
[   38.489433] x17: 0000000000000000 x16: 0000000000000000 x15: 0000000000000000
[   38.489662] x14: 0000000000000000 x13: 0000000000000000 x12: 0000000000000000
[   38.489894] x11: 0000000000000000 x10: 0000000000000000 x9 : 0000000000000000
[   38.490153] x8 : 0000000000000000 x7 : 0000000000000000 x6 : 0000000000000000
[   38.490383] x5 : 0000000000000001 x4 : ffffbf420d65f000 x3 : ffff8000102a3dc0
[   38.490614] x2 : 000000000000000c x1 : 0000000000000000 x0 : 0000000000000000
[   38.490944] Call trace:
[   38.491138]  faulty_write+0x14/0x20 [faulty]
[   38.491309]  ksys_write+0x74/0x110
[   38.491428]  __arm64_sys_write+0x20/0x30
[   38.491553]  invoke_syscall+0x48/0x110
[   38.491678]  el0_svc_common.constprop.0+0x44/0xf0
[   38.491827]  do_el0_svc+0x28/0x90
[   38.491938]  el0_svc+0x20/0x60
[   38.492053]  el0t_64_sync_handler+0x138/0x150
[   38.492194]  el0t_64_sync+0x1a0/0x1a4
[   38.492443] Code: d2800001 d2800000 d503233f d50323bf (b900003f) 
[   38.492827] ---[ end trace 97d1748abba7be4c ]---
