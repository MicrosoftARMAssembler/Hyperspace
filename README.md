# Hyperspace
Hyperspace is a multi purpose Windows Kernel Driver that has been in development since June 2025. <br />
I've applied Hyperspace for every purpose I've thought of **Injecting, Emulating, Dumping, Externals, e.g..** <br />
It's one of my biggest projects that I've ever developed, it's been in development for years and I hope you can enjoy. <br />

# What makes Hyperspace Undetected? 
Hyperspace was **designed to bypass Anti-Cheat Integrity Checks**. <br />
This was made possible through hours of Reverse-Engineering the industry-leading Anti-Cheat. (**EasyAntiCheat**) <br />
I will cover all of the fundamentals for what it's bypassing and the reason. <br />

<details>
  <summary><b>Manual Mapping</b></summary>
  
  <h3>What is Manual Mapping?</h3>
  Manual Mapping loads a kernel driver into <b>Ring 0 (Kernel Mode)</b> without using Windows' official driver loading <br />
  This bypasses all <b>DSE (driver signature enforcement)</b> checks and doesn't put it in kernel cache lists like <b>PsLoadedModuleList</b>. <br />
  
  <h3>How does the Manual Mapping work?</h3>
  The mapper exploits the <b>ASMMAP64</b> CVE, an undocumented CVE that has <b>never been publicly used</b> for mapping. <br />
  This gives us arbitrary kernel memory access to manual map and install hooks to execute kernel functions. <br />
  The mapper parses <b>NTOSKRNL's PDB symbols using the DIA SDK</b> to dynamically resolve all kernel functions. <br />
  This makes the driver completely <b>signature-less</b> and compatible with all Windows versions. <br />
  
  <h3>What makes it undetected?</h3>
  The driver is allocated using <b>MmAllocateIndependentPages</b>, which doesn't place the allocation in any cache lists. <br />
  This prevents anti-cheats like <b>EasyAntiCheat</b> from discovering the allocation through pool scanning. <br />
  After mapping, the allocation's base address and size are passed to the driver's entry point for page hiding and remapping.
  
</details>

<details>
  <summary><b>Page Hiding & Remapping</b></summary>
  
  <h3>Why Page Hiding and Remapping?</h3>
  Anti-cheats detect illegitimate drivers by scanning for <b>executable driver code</b> in kernel memory. <br />
  Different anti-cheats use different detection methods to find executing driver code. <br />
  
  <h3>What detection methods do anti-cheats use?</h3>
  <b>EasyAntiCheat</b> uses <b>VM instructions</b> to probe executing code in suspicious driver regions. <br />
  <b>EasyAntiCheat</b> and others use <b>NMIs (Non-Maskable Interrupts)</b> to catch illegitimate driver execution. <br />
  The solution is to either <b>hide</b> our driver's pages or <b>remap</b> them to appear legitimate. <br />
  
  <h3>How does Page Hiding and Remapping work?</h3>
  <b>Page Remapping</b> - <a href="./payload/src/paging/hyperspace/hyperspace.hxx">View Code</a><br />
  Maps our driver's page tables into a <b>legitimate kernel module's context</b> (like ntoskrnl.exe). <br />
  When anti-cheats see our driver's execution, they see it as coming from a trusted module. <br /><br />
  
  <b>Page Hiding</b> - <a href="./payload/src/paging/hide/hide.hxx">View Code</a><br />
  Prevents page table walking by blocking <b>MmCopyMemory</b> and similar APIs from accessing our driver's pages. <br />
  Anti-cheats attempting to read or scan our driver pages will fail, making our driver invisible to integrity checks. <br />
  Combined, these techniques prevent anti-cheats from detecting our driver's execution as illegitimate.
  
</details>

<details>
  <summary><b>Physical Memory Mapping</b></summary>
  
  <h3>What is Physical Memory Mapping?</h3>
  We use Physical Memory Mapping to <b>Read/Write Physical Memory</b> without relying on APIs like <b>MmCopyMemory</b> that could be detected. <br />
  For example <b>MmCopyMemory</b> is detected because it generates <b>Page Faults</b> that are hooked by Anti-Cheats. (For example <b>Vanguard</b>) <br />

  <h3>What are the Physical Memory Mapping methods?</h3>
  <b>DPM (Direct Physical Mapping)</b> - <a href="./payload/src/paging/dpm/dpm.hxx">View Code</a><br />
  DPM creates 64 allocated virtual pages (per CPU core) and dynamically remaps PTEs to target physical addresses. <br />
  When you need to read/write physical memory DPM changes the <b>PFN (Page Frame Number)</b> of the PTE. <br />
  This method is fast and perfect for quick read/write operations since pages are pre-allocated. <br /><br />
  
  <b>PTM (Page Table Mapping)</b> - <a href="./payload/src/paging/ptm/ptm.hxx">View Code</a><br />
  PTM constructs a custom page table hierarchy <b>(PML4, PDPT, PD, PT)</b> in an unused region of the address space. <br />
  It finds a free PML4 entry and builds new page tables, allowing you to map a physical page to a virtual address. <br />
  PTM can set the <b>user_supervisor</b> bit in page table entries, allowing <b>usermode to directly access the kernel memory</b>. <br />
  This method is better for when you need full control over virtual address layout and page protections. <br />

  <h3>Key Differences</h3>
  <b>DPM</b>: Reuses pre-allocated pages, per-CPU core isolation<br />
  <b>PTM</b>: Creates custom virtual address space, full page protection control, <b>usermode accessibility</b><br />
  Both bypass MmCopyMemory and while being invisible to all anti-cheat hooks.
  
</details>

<details>
  <summary><b>Client Communication</b></summary>
  
  <h3>Why Threadless Communication?</h3>
  Most commonly drivers communicate using <b>IOCTLs</b> or <b>kernel threads</b>, both of which are monitored by anti-cheats. <br />
  Anti-cheats scan for suspicious device objects, unnamed devices, and threads running in kernel mode from illegitimate modules. <br />
  Threadless communication eliminates all of these detection vectors entirely. <br />
  
  <h3>How does the communication work?</h3>
  <b>WNF (Windows Notification Facility)</b> - <a href="./payload/src/handler/handler.hxx">View Code</a><br />
  Uses <b>WNF state change subscriptions</b> to receive notifications without creating any threads. <br />
  WNF is a legitimate Windows mechanism, making our callbacks appear as normal activity. <br />
  This avoids touching any <b>suspicious cache lists</b> or calling <b>hooked functions</b> that anti-cheats monitor. <br /><br />
  
  <b>Process Callback Initialization</b> - <a href="./payload/src/handler/handler.hxx">View Code</a><br />
  A <b>process creation callback</b> detects when our client creates a process with a unique name. <br />
  Once detected, the driver attaches to the client process and creates a <b>Hyperspace (shared address space)</b>. <br />
  We <b>spoof the PTE flags</b> of our hyperspace allocation to allow the usermode direct access to kernel memory. <br />
  
  <b>How Does Our Client Communicate?</b> - <a href="./library/src/driver/driver.hxx">View Code</a><br />
  The client process writes request data to the shared hyperspace allocation and triggers the <b>WNF callback</b>. <br />
  Semaphores are used for synchronization - the client releases a semaphore to signal the request is ready. <br />
  The driver validates the semaphore, processes the request, then releases a response semaphore to notify completion. <br />  
</details>

# What can Hyperspace do?
Hyperspace is a diverse kernel driver with applications for emulation, injection, dumping, and external memory operations. <br />

<details>
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#EasyAntiCheat">EasyAntiCheat</a>
      <ul>
        <li><a href="#Emulation-Methods">Emulation Methods</a></li>
        <li><a href="#Internal-Functions">Internal Functions</a></li>
      </ul>
    </li>
    <li>
      <a href="#DLL-Injector">DLL Injector</a>
      <ul>
        <li><a href="#Dependency-Resolving">Dependency Resolving</a></li>
        <li><a href="#Injection-Cleanup">Injection Cleanup</a></li>
        <li><a href="#Allocation-Methods">Allocation Methods</a></li>
      </ul>
    </li>
    <li><a href="#Dumper">Dumper</a></li>
    <li><a href="#Roadmap">Roadmap</a></li>
    <li><a href="#Conclusion">Conclusion</a></li>
  </ol>
</details>

## EasyAntiCheat
Hyperspace (aka Angel-Engine Emulator) emulates EasyAntiCheat by hooking <b>KEP (Kernel Export Patch)</b> and <b>ETW (Event Windows Tracing)</b>. <br />
We additionally pattern scan EasyAntiCheat's driver to find internal functions, and <b>call them directly</b> instead of using APIs. <br />
This makes all operations appear as <b>EAC itself is performing them</b>, bypassing stack walking and integrity checks. <br />

#### Emulation Methods:
| Emulates | How does it work? |
|----------|----------|
| **EasyAntiCheat** | <details><summary>Blocks integrity checks via <b>KEP (Kernel Export Patch)</b> - <a href="./payload/src/mmu/core/kep">View Code</a></summary>Hooks kernel exports by patching ntoskrnl's <b>EAT (Export Address Table)</b> entries. <br />We allocate space inside ntoskrnl by <b>splitting large pages</b> to get a valid RVA for our shellcode. <br />Our shellcode is a trampoline that sets the stack as our hook handler. <br />We swap the export table RVA to point to our shellcode instead of the original function.</details> |
| **UserAntiCheat** | <details><summary>Blocks integrity checks via <b>ETW (Event Tracing Windows)</b> - <a href="./payload/src/mmu/core/etw">View Code</a></summary>Hooks the <b>CKCL (Circular Kernel Context Logger)</b> ETW session to intercept syscalls made by the usermode anti-cheat. <br />We configure CKCL to trace syscall events and hook <b>HalCollectPmcCounters</b> to capture syscalls. <br />When a syscall is caught, we walk the kernel stack looking for the magic values that mark ETW trace events. <br />When found, we swap the return address on the stack with our hook handler.</details> |

#### Internal Functions:
- Allocate Virtual - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L15-L24">View Code</a><br />
- Page Memory - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L26-L35">View Code</a><br />
- Free Virtual - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L37-L46">View Code</a><br />
- Flush Virtual - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L48-L57">View Code</a><br />
- Set Information Virtual - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L59-L68">View Code</a><br />
- Protect Virtual - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L70-L79">View Code</a><br />
- Query Virtual - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L81-L90">View Code</a><br />
- Read Virtual - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L92-L101">View Code</a><br />
- Write Virtual - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L103-L112">View Code</a><br />
- Attach Process - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L114-L123">View Code</a><br />
- Detach Process - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L136-L153">View Code</a><br />
- Create Callback - <a href="https://github.com/MicrosoftMacroAssembler/Angel-Engine-Emulator/blob/fbe87b1d5fbc8dffb8d4db36c0a08cdb705031d6/payload/src/mmu/core/eac/eac.hxx#L125-L134">View Code</a><br />

## DLL Injector
Hyperspace injects DLLs using reliable dependency resolution, and post-injection cleanup. <br />
Instead of hooking monitored **IDxgiSwapChain::Present**, we swap **DiscordHook64.dll's PeekMessage** hook. <br />
This bypasses anti-cheat heuristics that specifically watch for Direct3D/DXGI hooks.

### Dependency Resolving
Hyperspace uses a multi-layered fallback system to resolve DLL dependencies. <br />
This ensures **100% dependency resolution** across all scenarios:
1. **API Set Resolving** - Uses Windows API sets to resolve C runtime libraries via API scheming
2. **KnownDLLs Registry** - Checks the KnownDLLs registry key for system libraries
3. **Local Directory** - Searches the injector's local directory
4. **Game Directory** - Searches the target game's directory
5. **PDB Symbols** - Searches debug symbols to locate dependencies

### Injection Cleanup
After successful injection, Hyperspace cleans up to reduce detection:
* Erases **non-discardable PE sections** from memory
* Removes **allocated shellcode** used during injection

### Allocation Methods
Hyperspace dynamically selects a allocation method based on the target process. [View Code](./library/main.cxx#L516-L557)
| Method | When Used | Code |
|--------|-----------|------|
| **EAC Allocation** | For EasyAntiCheat-protected processes | [View Code](./library/main.cxx#L110-L120) |
| **Remap Allocation** | For processes not running with EasyAntiCheat | [View Code](./library/main.cxx#L494-L514) |
| **VAD Allocation** | A backup if you want to switch from Remap | [View Code](./library/main.cxx#L482-492) |

## Dumper
Hyperspace Dumper captures and decrypts pages from a target process protected by EasyAntiCheat. <br />
The dumper iterates through all pages in the target process, leveraging EAC to decrypt encrypted pages:

1. **Page Enumeration** - Scans all pages in the process image
2. **Encryption Detection** - Uses `hyperspace::eac::query_virtual` to check if pages are encrypted (`PAGE_NOACCESS`)
3. **Page Decryption** - For encrypted pages, calls EAC's internal functions to decrypt:
   - `page_memory` - Triggers page decryption
   - `flush_virtual` - Flushes the virtual memory
   - `protect_virtual` - Restores original page protection
   - `set_information_virtual` - Finalizes the page state
4. **Physical Mapping** - Translates virtual addresses to physical addresses via page table walking
5. **Page Capture** - Maps physical pages using PTM and copies decrypted data to output buffer

## Roadmap
- [x] Add Changelog
- [x] Finish Emulation
- [ ] Finish Dumper
- [ ] Add Thead Hijacking execution
- [ ] Finish KPP Bypass for different versions

## Conclusion
Hyperspace is a comprehensive kernel driver built to bypass anti-cheat integrity checks. <br />
Review the **source code** for detailed implementation and documentation. <br />
Found issues or want to contribute? Create an issue or contact me on Discord. <br />
