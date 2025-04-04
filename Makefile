CC = gcc
CFLAGS = -Wall -Wextra -O2
LDFLAGS = -lX11 -lXext -lXfixes -lXrender -lXshape
TARGET = xclock_overlay
SRCS = src/xclock_overlay.c
OBJS = $(SRCS:.c=.o)
DIST_DIR = dist

.PHONY: all clean install uninstall

all: $(DIST_DIR) $(DIST_DIR)/$(TARGET)

$(DIST_DIR):
	mkdir -p $(DIST_DIR)

$(DIST_DIR)/$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
	rm -rf $(DIST_DIR)

# Check OS type
OS_CHECK := $(shell ./scripts/detectos.sh)

install: $(DIST_DIR)/$(TARGET)
ifeq ($(OS_CHECK),el)
	install -D -m 755 $(DIST_DIR)/$(TARGET) $(DESTDIR)/usr/local/bin/$(TARGET)
else ifeq ($(OS_CHECK),suse)
	install -D -m 755 $(DIST_DIR)/$(TARGET) $(DESTDIR)/usr/local/bin/$(TARGET)
else
	install -D $(DIST_DIR)/$(TARGET) $(DESTDIR)/usr/local/bin/$(TARGET)
endif

uninstall:
	rm -f $(DESTDIR)/usr/local/bin/$(TARGET)
