// ProjectTwo.cpp
// CS 300 Project Two ABCU Advising Assistance Program
// Stores courses in a binary search tree and prints them in alphanumeric order.

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <utility>

using namespace std;

struct Course {
	string courseNumber;
	string courseName;
	vector<string> prerequisites;
};

// Remove whitespace surrounding a CSV field or user entry
string trim(const string& value) {
	size_t first = value.find_first_not_of(" \t\r\n");
	if (first == string::npos) return "";
	size_t last = value.find_last_not_of(" \t\r\n");
	return value.substr(first, last - first + 1);
}

// Course numbers are stored in uppercase for case-insensitice lookup.
string normalizeCourseNumber(string number) {
	number = trim(number);
	for (char& character : number) {
		character = static_cast<char>(toupper(static_cast<unsigned char>(character)));
	}
	return number;
}

struct TreeNode {
	Course course;
	TreeNode* leftChild;
	TreeNode* rightChild;
	explicit TreeNode(const Course& value)
		: course(value), leftChild(nullptr), rightChild(nullptr) {}
};

class CourseTree {
private:
	TreeNode* root;

	// Insert by course number; return false for a duplicate.
	bool insertTreeCourse(TreeNode*& node, const Course& course) {
		if (node == nullptr) {
			node = new TreeNode(course);
			return true;
		}
		if (course.courseNumber < node->course.courseNumber)
			return insertTreeCourse(node->leftChild, course);
		if (course.courseNumber > node->course.courseNumber)
			return insertTreeCourse(node->rightChild, course);
		return false;
	}

	const Course* findTreeCourse(const TreeNode* node, const string& number) const {
		if (node == nullptr)
			return nullptr;
		if (number == node->course.courseNumber)
			return &node-> course;
		if (number < node->course.courseNumber)
			return findTreeCourse(node->leftChild, number);
		return findTreeCourse(node->rightChild, number);
	}

	// In-order traversal prints course numbers from lowest to highest
	void printTreeInOrder(const TreeNode* node) const {
		if (node == nullptr)
			return;
		printTreeInOrder(node->leftChild);
		cout << node->course.courseNumber << ", " << node->course.courseName << '\n';
		printTreeInOrder(node->rightChild);
	}

	void deleteTree(TreeNode* node) {
		if (node == nullptr)
			return;
		deleteTree(node->leftChild);
		deleteTree(node->rightChild);
		delete node;
	}

public:
	CourseTree() : root(nullptr) {}
	~CourseTree() {
		clear();
	}
	CourseTree(const CourseTree&) = delete;
	CourseTree& operator=(const CourseTree&) = delete;

	void clear() {
		deleteTree(root);
		root = nullptr;
	}

	bool insert(const Course& course) {
		return insertTreeCourse(root, course);
	}
	const Course* find(const string& number) const {
		return findTreeCourse(root, normalizeCourseNumber(number));
	}
	void printAll() const {
		printTreeInOrder(root);
	}
	void swap(CourseTree& other) {
		std::swap(root, other.root);
	}
};


// Load into a temporary tree so an invalid file never replaces valid data
bool loadTreeCourses(const string& fileName, CourseTree& courseTree) {
	ifstream file(fileName.c_str());
	if (!file.is_open()) {
		cout << "Error: Unable to open course file: " << fileName << '\n';
		return false;
	}

	CourseTree newTree;
	vector<Course> allCourses;
	string line;
	size_t lineNumber = 0;

	while (getline(file, line)) {
		++lineNumber;
		if (trim(line).empty())
			continue;

		vector<string> tokens;
		string token;
		stringstream lineStream(line);
		while (getline(lineStream, token, ','))
			tokens.push_back(trim(token));

		if (tokens.size() < 2 || tokens[0].empty() || tokens[1].empty()) {
			cout << "Error: Line " << lineNumber
					<< " must contain a course number and a course name.\n";
			return false;
		}

		Course currentCourse;
		currentCourse.courseNumber = normalizeCourseNumber(tokens[0]);
		currentCourse.courseName = tokens[1];

		// Blank trailing CSV fields mean no additional prerequisites.
		for (size_t i = 2; i < tokens.size(); ++i) {
			if (!tokens[i].empty())
				currentCourse.prerequisites.push_back(normalizeCourseNumber(tokens[i]));
		}

		if (!newTree.insert(currentCourse)) {
			cout << "Error: Duplicate course number " << currentCourse.courseNumber
					<< " on line " << lineNumber << ".\n";
			return false;
		}
		allCourses.push_back(currentCourse);
	}

	if (allCourses.empty()) {
		cout << "Error: The course file contains no courses.\n";
		return false;
	}

	// Verify prerequisites only after every course has been inserted.
	for (const Course& course : allCourses) {
		for (const string& prerequisite : course.prerequisites) {
			if (newTree.find(prerequisite) == nullptr) {
				cout << "Error: Prerequisite " << prerequisite << " for course "
						<< course.courseNumber << " does not exist in the course file.\n";
				return false;
			}
		}
	}

	courseTree.swap(newTree);
	cout << "Course data loaded successfully (" << allCourses.size() << " courses).\n";
	return true;
}

void printCourse(const CourseTree& courseTree, const string& number) {
	const Course* course = courseTree.find(number);
	if (course == nullptr) {
		cout << "Course not found.\n";
		return;
	}

	cout << course->courseNumber << ", " << course->courseName << '\n';
	if (course->prerequisites.empty()) {
		cout << "Prerequisites: None\n";
	}
	else {
		cout << "Prerequisites: ";
		for (size_t i = 0; i < course->prerequisites.size(); ++i) {
			const Course* prerequisite = courseTree.find(course->prerequisites[i]);
		if (i > 0)
			cout << "; ";
		if (prerequisite != nullptr)
			cout << prerequisite->courseNumber << " (" << prerequisite->courseName << ")";
		}
		cout << '\n';
	}
}

void displayMenu() {
	cout << "Menu:\n"
			<< "  1. Load Data Structure.\n"
			<< "  2. Print Course List.\n"
			<< "  3. Print Course.\n"
			<< "  9. Exit.\n\n"
			<< "Please select an option.";
}

int main() {
	CourseTree courseTree;
	bool dataLoaded = false;
	string choice;

	cout << "Welcome to the Course Planner.\n";
	while (true) {
		displayMenu();
		if (!getline(cin, choice))
			break;
		choice = trim(choice);

		// Only a single character can be a valid menu selection.
		char option = (choice.length() == 1) ? choice[0] : '\0';

		switch (option) {
			case 1:
				string fileName;
				cout << "Enter the course data file name: ";
				if (!getline(cin, fileName))
					break;
				fileName = trim(fileName);
				if (fileName.empty()) {
					cout << "Error: File name cannot be empty.\n";
				}
					break;

			case 2:
				if (!dataLoaded) {
					cout << "Error: Please load course data first (option 1).\n";
				}
				else {
					cout << "\nCourse List:\n\n";
					courseTree.printAll();
				}
				break;

			case 3:
				if (!dataLoaded) {
					cout << "Error: Please load course data first (option 1).\n";
				}
				else {
					string courseNumber;
					cout << "Enter course number: ";
					if (!getline(cin, courseNumber))
						break;
					if (trim(courseNumber).empty())
						cout << "Error: COurse number cannot be empty.\n";
					else
						printCourse(courseTree, courseNumber);
				}
				break;
			}
			case 9:
				cout << "Thank you for using the course planner!\n";
				return 0;

			default:
				cout << choice << " is not a valid option.\n;";
				break;
		}
	cout << "Thank you for using the course planner!\n";
	return 0;
}
