# Combinator Reduction Visualizer  
*A Functional Programming Project in Haskell*

## Overview

This project is a Haskell-based visualizer for step-by-step reduction of combinator expressions. It was developed as part of a functional programming course and explores the combinatory logic behind symbolic reduction using core combinators such as `S`, `K`, `I`, and `B`.

Combinatory logic is a formal system for eliminating variables in mathematical logic. This tool allows you to trace how expressions reduce to their normal form by applying predefined reduction rules.

## What It Does

Given a combinator expression built from:

- **Primitive combinators**: `S`, `K`, `I`, `B`
- **Variables**: `X`, `Z`, or indexed variables `V Int`
- **Application**: using the left-associative constructor `:$`

The program provides:

1. A **pretty-printer** that displays expressions without unnecessary parentheses (`prettyExpr`).
2. A function to perform a **single-step reduction** (`rstep`).
3. A function to compute the **full reduction path** until the normal form (`rpath`).
4. A function to **print** the step-by-step reduction process (`printPath`).

## Example Usage

```haskell
ghci> prettyExpr omega
"S I I (S I I)"

ghci> printPath test1
S K K x
K x (K x)
x

ghci> printPath kio
K I (S I I (S I I))
I
```

