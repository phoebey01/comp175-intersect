CPP = "g++"
CPPFLAGS =
GLFLAGS = -lglut -lglui -lGL -lGLU
MACFLAGS = -framework OpenGL -framework GLUT -lglui
XMLPARSE = tinystr.cpp tinyxml.cpp tinyxmlerror.cpp tinyxmlparser.cpp


mac:
	$(CPP) $(CPPFLAGS) Assignment4.cpp Camera.cpp SceneParser.cpp $(XMLPARSE) $(MACFLAGS) -o a4

linux:
	$(CPP) $(CPPFLAGS) Assignment4.cpp Camera.cpp SceneParser.cpp $(XMLPARSE) $(GLFLAGS) -o a4

clean:
	rm -rf *.o a4
