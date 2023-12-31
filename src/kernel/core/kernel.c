#include <configs.h>
#include <drivers/serial/serial.h>
#include <kernel/arch/x64/vga.h>
#include <kernel/arch/x64/idt.h>
#include <kernel/mem/pmm.h>
#include <utils/log.h>
#include <klib/stdio.h>
#include <multiboot.h>
#include <drivers/io/ports.h>
#include <drivers/video/framebuffer.h>

extern uint64_t _kernel_start;
extern uint64_t _kernel_physical_end;
extern uint64_t _kernel_end;
extern uint64_t _kern_virtual_offset;

extern uint64_t multiboot_framebuffer_data;
extern uint64_t multiboot_basic_meminfo;
extern uint64_t multiboot_mmap_data;

struct multiboot_tag_basic_meminfo *tagmem = NULL;
struct multiboot_tag_framebuffer *tagfb = NULL;
struct multiboot_tag_mmap *tagmmap = NULL;

extern uint64_t p2_table[];
extern uint64_t p4_table[];
extern uint64_t p3_table[];
extern uint64_t p3_table_hh[];

// extern uint64_t pt_tables[];
extern uint64_t end_of_mapped_memory;

uint64_t memory_size_in_bytes = 0;

void basic_system_init(unsigned long addr)
{

    struct multiboot_tag *tag;
    uint32_t mbi_size = *(uint32_t *)(addr + _HIGHER_HALF_KERNEL_MEM_START);

    tagmem = (struct multiboot_tag_basic_meminfo *)(multiboot_basic_meminfo + _HIGHER_HALF_KERNEL_MEM_START);
    tagfb = (struct multiboot_tag_framebuffer *)(multiboot_framebuffer_data + _HIGHER_HALF_KERNEL_MEM_START);
    tagmmap = (struct multiboot_tat_mmap *)(multiboot_mmap_data + _HIGHER_HALF_KERNEL_MEM_START);

    memory_size_in_bytes = (tagmem->mem_upper + 1024) * 1024;

    klog("Mem: %u MB (%u KB)\n", memory_size_in_bytes / 1024 / 1024, memory_size_in_bytes / 1024);

    framebuffer_init(tagfb->common.framebuffer_addr + _HIGHER_HALF_KERNEL_MEM_START);

    // klog("End of mapped mem: 0x%Z\n", end_of_mapped_memory);
    mmap_parse(tagmmap);
    pmm_setup(addr, mbi_size);

    mmap_setup();

    /*

    klog("mapped tables:\n");
    klog("P4:\n");
    for (int i = 0; i < 512; i++)
    {
        if (p4_table[i] != 0)
            klog("%i: 0x%Z\n", i, (uint64_t)p4_table[i]);
    }

    klog("P3:\n");
    for (int i = 0; i < 1024; i++)
    {
        if (p3_table[i] != 0)
            klog("%i: 0x%Z\n", i, (uint64_t)p3_table[i]);
    }

    klog("P3 hh:\n");
    for (int i = 0; i < 1024; i++)
    {
        if (p3_table_hh[i] != 0)
            klog("%i: 0x%Z\n", i, (uint64_t)p3_table_hh[i]);
    }

    klog("P2:\n");
    for (int i = 0; i < 1024; i++)
    {
        if (p2_table[i] != 0)
            klog("%i: 0x%Z\n", i, (uint64_t)p2_table[i]);
    }
    */

    /*
        klog("Pt tables:\n");
        for (int i = 0; i < 1024; i++)
        {
            if (pt_tables[i] != 0)
                klog("%i: 0x%Z\n", i, (uint64_t)pt_tables[i]);
        }
        */
}

void cursor_disable()
{
    outportb(0x3d4, 0x0A);
    outportb(0x3d5, 0x20);
}

void kernel_start(unsigned long addr, unsigned long magic)
{
    print_clear();
    print_set_color(COLOR_LIGHT_GRAY, COLOR_BLACK);

    serial_init();

    klog("PotatOS\n");

    uint64_t kernelStart = (uint64_t)&_kernel_start;
    uint64_t kernelEnd = (uint64_t)&_kernel_physical_end;
    uint64_t kernelSize = (((unsigned long)&_kernel_end + (1024 * 1024)) - (unsigned long)&_kern_virtual_offset);

    // klog("virtual offset: %Z\n", (unsigned long)&_kern_virtual_offset);

    unsigned size = *(unsigned *)(addr + _HIGHER_HALF_KERNEL_MEM_START);

    if (magic = 0x36d76289)
    {
        // klog("Magic number verified Size:  %X - Magic: %X\n", size, magic);
    }
    else
    {
        klog("Failed to verify magic number. Something is wrong\n");
    }

    idt_init();

    basic_system_init(addr);

    cursor_disable();

    if (tagfb != NULL && tagfb->common.size == 38)
    {
        int k = 0;
        int speed = 1;
        int dx = speed;
        int dy = speed;

        int x = tagfb->common.framebuffer_width / 2;
        int y = tagfb->common.framebuffer_height / 2;

        while (1)
        {
            framebuffer_clear_black();
            // draw_testCanvas();

            draw_text(x, y, 255, 255, 255, "Ohai!");

            x += dx;
            y += dy;

            if (x > tagfb->common.framebuffer_width - 100)
            {
                dx = -speed;
            }
            if (x <= 0)
            {
                dx = speed;
            }
            if (y > tagfb->common.framebuffer_height - 20)
            {
                dy = -speed;
            }
            if (y <= 0)
            {
                dy = speed;
            }

            framebuffer_flip();
        }
    }

    while (1)
        ;
}
