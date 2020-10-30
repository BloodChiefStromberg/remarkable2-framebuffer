#include "../shared/ipc.cpp"

#include <stdlib.h>

int msg_q_id = 0x2257c;
ipc::Queue MSGQ(msg_q_id);

uint32_t WIDTH = 1404;
uint32_t HEIGHT = 1872;

class SwtFB {
public:
  uint16_t *fbmem;
  ipc::swtfb_update update;
  ipc::swtfb_rect dirty_area;

  SwtFB() {
    fbmem = ipc::get_shared_buffer();
    reset_dirty();
  }

  void reset_dirty() {
    dirty_area.left = WIDTH;
    dirty_area.top = HEIGHT;
    dirty_area.width = 0;
    dirty_area.height = 0;
  }

  void mark_dirty(ipc::swtfb_rect &&rect) { mark_dirty(rect); }

  void mark_dirty(ipc::swtfb_rect &rect) {

    cout << "DIRTY" << rect.left << " " << rect.top << " " << rect.width << " " << rect.height << endl;
    uint32_t x1 = dirty_area.left + dirty_area.width;
    uint32_t y1 = dirty_area.top + dirty_area.height;

    x1 = max(x1, rect.left + rect.width);
    y1 = max(y1, rect.top + rect.height);

    if (x1 > WIDTH) {
      x1 = WIDTH-1;
    }
    if (y1 > HEIGHT) {
      y1 = HEIGHT-1;
    }

    dirty_area.left = min(rect.left, dirty_area.left);
    dirty_area.top = min(rect.top, dirty_area.top);

    dirty_area.width = x1 - dirty_area.left;
    dirty_area.height = y1 - dirty_area.top;
    cout << "DIRTY" << dirty_area.left << " " << dirty_area.top << " " << dirty_area.width << " " << dirty_area.height << endl;
  }

  void redraw_screen(bool full_refresh = false) {
    ipc::swtfb_update update;
    if (full_refresh || dirty_area.width <= 0 || dirty_area.height <= 0) {
      ipc::swtfb_rect buf = {};
      buf.left = WIDTH;
      buf.top = HEIGHT;
      buf.width = 0;
      buf.height = 0;
      update.update_region = buf;
    } else {
      update.update_region = dirty_area;
    }

    MSGQ.send(update);
    reset_dirty();
  }
};

int main() {
  srand(time(NULL));
  printf("SENDING MSG UPDATE\n");

  SwtFB fb;

  int offset = (rand() % 1024);

  for (unsigned int i = 0; i < WIDTH*HEIGHT; i++) {
    fb.fbmem[i] = i + offset;
  }

  uint32_t x = (rand() % WIDTH);
  uint32_t y = (rand() % HEIGHT);
  if (x > WIDTH) { x -= WIDTH; };
  if (y > HEIGHT) { y -= HEIGHT; };
  uint32_t w = 200 + (rand() % 10+1) * 50;
  uint32_t h = 200 + (rand() % 10+1) * 50;

  cout << x << " " << y << " " << w << " " << h << endl;
  fb.mark_dirty({.left=x, .top=y, w, h});
  fb.redraw_screen();
}
