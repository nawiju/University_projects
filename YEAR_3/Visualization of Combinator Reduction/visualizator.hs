-- Autor: Natalia Junkiert
-- UID: nj448267

data Expr = S | K | I | B 
          | Expr :$ Expr 
          | X | Z | V Int  
          deriving (Show, Eq)

-- Pretty printing of expressions
prettyExpr :: Expr -> String
-- Base cases; S, K, I, B, X, Z, V n are printed as strings
prettyExpr S = "S"
prettyExpr K = "K"
prettyExpr I = "I"
prettyExpr B = "B"
prettyExpr X = "x"
prettyExpr Z = "z"
-- V n is printed as "v" followed by the number n; handles cases like (V 7)
prettyExpr (V n) = "v" ++ show n
-- e1 is applied to e2; handles cases like (S :$ K :$ K)
prettyExpr (e1 :$ e2) = prettyExpr e1 ++ " " ++ prettyArg e2
-- Helper function to pretty print the argument of an expression
  where
    prettyArg e@(_ :$ _) = "(" ++ prettyExpr e ++ ")" -- if it is a compound expression, put it in parentheses
    prettyArg e = prettyExpr e -- otherwise, just print it

-- Perform one step of reduction
rstep :: Expr -> Expr
-- Reduction rules; written in the assignment description
rstep (S :$ x :$ y :$ z) = x :$ z :$ (y :$ z)
rstep (K :$ x :$ y) = x
rstep (I :$ x) = x
rstep (B :$ x :$ y :$ z) = x :$ (y :$ z)
-- If the expression is a compound expression, reduce the leftmost redex first
-- If the leftmost redex is not reducible, reduce the rightmost redex
-- Apply either e1' or e1 to e2, depending on whether e1' is reducible or not   
rstep (e1 :$ e2) = case rstep e1 of
  e1' -> if e1' == e1 then e1 :$ rstep e2 else e1' :$ e2  -- Reduce leftmost, then right
rstep e = e  -- No redex found

-- Optimization (prioritize shortening redexes)
rstep' :: Expr -> Expr
rstep' e =
  -- First reduce the expression before moving on 
  let e' = reduceShortening e  
  in if e' == e -- If the expression is not reducible
     then rstep e -- Reduce the expression
     else e' -- Otherwise, return the reduced expression

-- Reduce all shortening redexes in the expression
reduceShortening :: Expr -> Expr
-- Reduction rules for shortening redexes
-- If it is an I expression, just shorten it, etc
reduceShortening (I :$ x) = x  
reduceShortening (K :$ x :$ y) = x  
reduceShortening (B :$ x :$ y :$ z) = x :$ (y :$ z) 
-- Function application
reduceShortening (e1 :$ e2) =
  -- Recursively reduce e1 and e2
  let e1' = reduceShortening e1
      e2' = reduceShortening e2
  in if e1' == e1 && e2' == e2 -- if the reduction changed nothing then return the original expression
     then e1 :$ e2
     else reduceShortening (e1' :$ e2') -- otherwise, reduce the new expression again
reduceShortening e = e -- If it did not match then return the original expression

-- Generate the reduction path, limit of 30 stepsnew
rpath :: Expr -> [Expr]
-- applies iterateWithOriginal to the expression and the rstep function
rpath e = take 30 $ iterateWithOriginal e rstep
  where
    -- Recursively apply the function f to the expression e until it does not change
    iterateWithOriginal e f = e : if e == f e then [] else iterateWithOriginal (f e) f

rpath' :: Expr -> [Expr]
-- applies iterateWithOriginal to the expression and the rstep' function
rpath' e = take 30 $ iterateWithOriginal e rstep'
  where
    -- Recursively apply the function f to the expression e until it does not change
    iterateWithOriginal e f = e : if e == f e then [] else iterateWithOriginal (f e) f

-- Print the reduction path
printPath :: Expr -> IO ()
-- Get the reduction path
-- Iterate using mapM_ over the path and print each expression
printPath e = mapM_ (putStrLn . prettyExpr) (rpath e)

printPath' :: Expr -> IO ()
-- Same as printPath but uses rpath' instead of rpath
printPath' e = mapM_ (putStrLn . prettyExpr) (rpath' e)

-- Tests
test1 = S :$ K :$ K :$ X
twoB = S :$ B :$ I
threeB = S :$ B :$ (S :$ B :$ I)
test3 = threeB :$ X :$ Z
omega = ((S :$ I) :$ I) :$ ((S :$ I) :$ I)
kio = K :$ I :$ omega
add = (B :$ S) :$ (B :$ B)

