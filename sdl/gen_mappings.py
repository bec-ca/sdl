#!/usr/bin/env python3

def main():
    for code in range(ord('a'), ord('z')+1):
        print(f'{{ SDLK_{chr(code)}, KeyCode::{chr(code).upper()} }},')



main()
