/* Copyright (c) 2021, Robert Harris. */

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public class Solution {
	public static int highest(List<Integer> a, List<Integer> b,
	  List<Integer> c) {
		List<Integer> all = new ArrayList<Integer>();
		all.addAll(a);
		all.addAll(b);
		all.addAll(c);
		Collections.sort(all);

		int sum = 0;
		int column = 1;
		for (Integer i : all) {
			sum += i * column;
			column *= 10;
		}
		
		return sum;
	}
	
	// The following solution relies on some elementary number theory, starting
	// with the concept of a "digital root", discussed by the popular science
	// writer Martin Gardner in something like "Mathematics for the Millions"
	// (the exact details are hazy:  it's been a while).  The digital root, D,
	// of a number N is the sum of N's digits.  D has the interesting property
	// that N mod 9 = D mod 9.  Clearly, N mod 3 = D mod 3 also.  It follows
	// that whether some number N is divisible by 3 is independent of the
	// permutation of its digits.  Thus if N is some integer formed by the
	// digits in the input set S and D mod 3 = 1 or D mod 3 = 2 then we must
	// remove elements from S until the condition is met.
	//
	// Removal is guided by placing each element e of S in one of three subsets,
	// R0, R1 and R2, according to the result of e mod 3.  If D mod 3 is 1 then
	// removing a single element from R1 would restore divisibility of D, and
	// therefore N, by 3.  Clearly, removing the smallest element will maximise
	// the value that may be formed with the remaining elements.  If R1 is empty
	// then the choice is to take the two smallest elements from R2.
	//
	// If D mod 3 = 2 then, similarly, we take the smallest element of R2 or,
	// if R2 is empty, the two smallest elements from R1.
	//
	// Finally, the remaining elements are sorted in descending order to yield
	// the largest value of N.
	public static int solution(int[] l) {
		List<Integer> R0 = new ArrayList<Integer>();	
		List<Integer> R1 = new ArrayList<Integer>();	
		List<Integer> R2 = new ArrayList<Integer>();

		int dr = 0;
		for (Integer i : l) {
			dr += i;
			switch (i % 3) {
			case 0:
				R0.add(i);
				break;
			case 1:
				R1.add(i);
				break;
			case 2:
				R2.add(i);
				break;
			}				
		}	
		Collections.sort(R0);
		Collections.sort(R1);
		Collections.sort(R2);

		switch (dr % 3) {
		case 0:
			break;
		case 1:
			if (R1.size() > 0) {
				R1.remove(0);			
			} else if (R2.size() > 1) {
				R2.remove(0);
				R2.remove(0);
			} else {
				return 0;
			}
			break;
		case 2:
			if (R2.size() > 0) {
				R2.remove(0);			
			} else if (R1.size() > 1) {
				R1.remove(0);
				R1.remove(0);
			} else {
				return 0;
			}
			break;
		}

		return highest(R0, R1, R2);		
	}
}
