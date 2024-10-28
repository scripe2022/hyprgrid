#!/usr/bin/python3
# run := python3 show.py
# dir  := .
# kid  :=

from PIL import Image, ImageDraw
import math

def create_monitor_and_draw_windows(monitor_width, monitor_height):
    img = Image.new("RGB", (monitor_width, monitor_height), "white")
    draw = ImageDraw.Draw(img)

    return img, draw

def draw_window(img, draw, top_left_x, top_left_y, window_width, window_height):
    bottom_right_x = top_left_x + window_width
    bottom_right_y = top_left_y + window_height
    draw.rectangle([top_left_x, top_left_y, bottom_right_x, bottom_right_y], outline="black", fill="black")

monitor_width, monitor_height = 2102, 2144
num_of_windows = 2
ratio = 16 / 9
gaps = 50

img, draw = create_monitor_and_draw_windows(monitor_width, monitor_height)

def manage(monitor_width, monitor_height, num_of_windows, ratio, gaps, img, draw):
    maxw = -1
    maxid = -1
    sw = -1
    sh = -1
    for i in range(1, num_of_windows, 1):
        w = math.floor(((monitor_width + gaps) / i) - gaps)
        h = math.floor(w / ratio)
        rows = math.ceil(num_of_windows / i)
        if h * rows + gaps * (rows - 1) > monitor_height:
            continue
        if w > maxw:
            maxw = w
            maxid = i
            sw = w
            sh = h
    for i in range(1, num_of_windows + 1, 1):
        h = math.floor(((monitor_height + gaps) / i) - gaps)
        w = math.floor(h * ratio)
        cols = math.ceil(num_of_windows / i)
        if w * cols + gaps * (cols - 1) > monitor_width:
            continue
        if w > maxw:
            maxw = w
            maxid = cols
            sw = w
            sh = h
    cols = maxid
    rows = math.ceil(num_of_windows / cols)
    tw = cols * sw + gaps * (cols - 1)
    th = rows * sh + gaps * (rows - 1)
    tl_x, tl_y = math.floor((monitor_width - tw) / 2), math.floor((monitor_height - th) / 2)
    for i in range(0, rows - 1, 1):
        for j in range(0, cols, 1):
            draw_window(img, draw, tl_x + j * (sw + gaps), tl_y + i * (sh + gaps), sw, sh)
    tw_last = (num_of_windows - (rows - 1) * cols) * sw + gaps * (num_of_windows - (rows - 1) * cols - 1)
    tl_x_last = math.floor((monitor_width - tw_last) / 2)
    for j in range(0, num_of_windows - (rows - 1) * cols, 1):
        draw_window(img, draw, tl_x_last + j * (sw + gaps), tl_y + (rows - 1) * (sh + gaps), sw, sh)

def manage_bs(monitor_width, monitor_height, num_of_windows, ratio, gaps, img, draw):
    left = 1
    right = num_of_windows
    win_width = -1
    win_height = -1
    win_per_row = -1
    win_per_col = -1

    while left <= right:
        wpr = (left + right) // 2
        print(wpr, "wpr")
        rows = math.ceil(num_of_windows / wpr)
        w = math.floor(((monitor_width + gaps) / wpr) - gaps)
        h = math.floor(w / ratio)
        total_height = (h + gaps) * rows - gaps
        if total_height <= monitor_height:
            if w > win_width:
                win_width = w
                win_height = h
                win_per_row = wpr
                win_per_col = rows
            right = wpr - 1
        else:
            left = wpr + 1
    # left = 1
    # right = num_of_windows
    # while left <= right:
    #     wpc = (left + right) // 2
    #     cols = math.ceil(num_of_windows / wpc)
    #     h = math.floor(((monitor_height + gaps) / wpc) - gaps)
    #     w = math.floor(h * ratio)
    #     total_width = (w + gaps) * cols - gaps
    #     if total_width <= monitor_width:
    #         if h > win_height:
    #             win_width = w
    #             win_height = h
    #             win_per_row = cols
    #             win_per_col = wpc
    #         right = wpc - 1
    #     else:
    #         left = wpc + 1
    # print
    print(win_width, win_height, win_per_row, win_per_col, "@")

    result = []
    top_offset_base = (monitor_height - win_per_col * (win_height + gaps) + gaps) // 2
    for i in range(win_per_col):
        n = win_per_row if i != win_per_col - 1 else num_of_windows - i * win_per_row
        row_len = n * (win_width + gaps) - gaps
        left_offset_base = (monitor_width - row_len) // 2
        top_offset = top_offset_base + i * (win_height + gaps)
        for j in range(n):
            left_offset = left_offset_base + j * (win_width + gaps)
            result.append((left_offset, top_offset, win_width, win_height))
    for item in result:
        draw_window(img, draw, item[0], item[1], item[2], item[3])

# manage(monitor_width, monitor_height, num_of_windows, ratio, gaps, img, draw)
manage_bs(monitor_width, monitor_height, num_of_windows, ratio, gaps, img, draw)
img.show()

