//UMN CSCI 5607 2D Geometry Library Homework [HW0]
//TODO: For the 18 functions below, replace their sub function with a working version that matches the desciption.

//Student: Eliza MacLean macle119

#ifndef GEOM_LIB_H
#define GEOM_LIB_H

#include "PGA/pga.h"

//Displace a point p on the direction d
//The result is a point
Point2D move(Point2D p, Dir2D d){
	return (Point2D) (p + d);
}

//Compute the displacement vector between points p1 and p2
//The result is a direction 
Dir2D displacement(Point2D p1, Point2D p2){
	return (Dir2D) p1 - p2;
}

//Compute the distance between points p1 and p2
//The result is a scalar 
float dist(Point2D p1, Point2D p2){
	return displacement(p1, p2).magnitude();
}

//Compute the perpendicular distance from the point p the the line l
//The result is a scalar 
float dist(Line2D l, Point2D p){
	return MultiVector(p).normalized().vee(MultiVector(l).normalized()).s;
}

//Compute the perpendicular distance from the point p the the line l
//The result is a scalar 
float dist(Point2D p, Line2D l){
	return MultiVector(p).normalized().vee(MultiVector(l).normalized()).s;
}

//Compute the intersection point between lines l1 and l2
//You may assume the lines are not parallel
//The results is a a point that lies on both lines
Point2D intersect(Line2D l1, Line2D l2){
	return Point2D(MultiVector(l1).wedge(MultiVector(l2)));
}

//Compute the line that goes through the points p1 and p2
//The result is a line 
Line2D join(Point2D p1, Point2D p2){
	return Line2D(MultiVector(p1).vee(MultiVector(p2)));
}

//Compute the projection of the point p onto line l
//The result is the closest point to p that lies on line l
Point2D project(Point2D p, Line2D l){
	return Point2D(MultiVector(l).dot(p).times(MultiVector(l)));
}

//Compute the projection of the line l onto point p
//The result is a line that lies on point p in the same direction of l
Line2D project(Line2D l, Point2D p){
	return Line2D(MultiVector(l).dot(p).times(MultiVector(p)));
}

//Compute the angle point between lines l1 and l2 in radians
//You may assume the lines are not parallel
//The results is a scalar
float angle(Line2D l1, Line2D l2){
	return std::acos(MultiVector(l1).normalized().dot(MultiVector(l2).normalized()).magnitude());
}

//Compute if the line segment p1->p2 intersects the line segment a->b
//The result is a boolean
bool segmentSegmentIntersect(Point2D p1, Point2D p2, Point2D a, Point2D b){
	MultiVector l1 = MultiVector(p1).vee(MultiVector(p2));
	MultiVector l2 = MultiVector(a).vee(MultiVector(b));
	float l1s = l1.vee(MultiVector(a)).s * l1.vee(MultiVector(b)).s;
	float l2s = l2.vee(MultiVector(p1)).s * l2.vee(MultiVector(p2)).s;
	return l1s < 0 && l2s < 0; 
}

//Compute if the point p lies inside the triangle t1,t2,t3
//Your code should work for both clockwise and counterclockwise windings
//The result is a bool
bool pointInTriangle(Point2D p, Point2D t1, Point2D t2, Point2D t3){
	MultiVector lines[3];
	lines[0] = MultiVector(t1).vee(t2);
	lines[1] = MultiVector(t2).vee(t3);
	lines[2] = MultiVector(t3).vee(t1);

	bool sides[3];
	sides[0] = MultiVector(p).vee(lines[0]).s > 0;
	sides[1] = MultiVector(p).vee(lines[1]).s > 0;
	sides[2] = MultiVector(p).vee(lines[2]).s > 0;

	return (sides[0] == sides[1]) && (sides[1] == sides[2]);
}

//Compute the area of the triangle t1,t2,t3
//The result is a scalar
float areaTriangle(Point2D t1, Point2D t2, Point2D t3){
	return 0.5 * (MultiVector(t1).normalized().vee(MultiVector(t2).normalized()).vee(MultiVector(t3).normalized()).s);
}

//Compute the distance from the point p to the triangle t1,t2,t3 as defined 
//by it's distance from the edge closest to p.
//The result is a scalar
//NOTE: There are some tricky cases to consider here that do not show up in the test cases!
float pointTriangleEdgeDist(Point2D p, Point2D t1, Point2D t2, Point2D t3){

	return 0; //Wrong, fix me...
}

//Compute the distance from the point p to the closest of three corners of
// the triangle t1,t2,t3
//The result is a scalar
float pointTriangleCornerDist(Point2D p, Point2D t1, Point2D t2, Point2D t3){
	return 0; //Wrong, fix me...
}

//Compute if the quad (p1,p2,p3,p4) is convex.
//Your code should work for both clockwise and counterclockwise windings
//The result is a boolean
bool isConvex_Quad(Point2D p1, Point2D p2, Point2D p3, Point2D p4){
	return false; //Wrong, fix me...
}

//Compute the reflection of the point p about the line l
//The result is a point
Point2D reflect(Point2D p, Line2D l){
	return Point2D(0,0); //Wrong, fix me...
}

//Compute the reflection of the line d about the line l
//The result is a line
Line2D reflect(Line2D d, Line2D l){
	return Line2D(0,0,0); //Wrong, fix me...
}

#endif
