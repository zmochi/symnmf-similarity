import numpy as np
import pandas as pd
import sys
import symnmfmodule


def main():
    k = sys.argv[1]
    goal = sys.argv[2]
    text_name = sys.argv[3]
    points = parse_points(text_name)
    k = parse_k(k, len(points))
    parse_goal(k, goal, points)


def parse_cmdline_arg(arg_str):
    try:
        parsed_arg = float(arg_str)
    except ValueError:
        return -1  # indicate error with -1 since commandline args shouldn't be negative

    if parsed_arg - int(parsed_arg) != 0:
        return -1  # indicate error with -1 since commandline args shouldn't be negative
    return int(parsed_arg)


def parse_points(text_name):
    points = []
    type = text_name[-4:]
    if not (type == ".txt"):
        print("An Error Has Occurred")
        exit(1)
    f = open(text_name, "r")
    for x in f:
        x = x.replace("\n", "")
        pointstr = x.split(",")
        point = []
        for str in pointstr:
            point.append(float(str))
        points.append(point)
    return points


def parse_k(k, len_points):
    k = parse_cmdline_arg(k)
    if not (k > 1 and k < len_points):
        print("Invalid number of clusters!")
        exit(1)
    return k


def init_H():
    print("H")


def parse_goal(k, goal, points):
    matrix = []
    if goal == "symnmf":
        print("symnmf")
    elif goal == "sym":
        matrix = symnmfmodule.sym(points)
    elif goal == "ddg":
        matrix = symnmfmodule.ddg(points)
    elif goal == "norm":
        matrix = symnmfmodule.norm(points)
    output_matrix(matrix)


def output_matrix(matrix):
    line = ""
    for k in matrix:
        for i in range(len(k)):
            formated_num = "%0.4f" % k[i]
            if i != len(k)-1:
                line += formated_num+","
            else:
                line += formated_num
        print(line)
        line = ""


main()
