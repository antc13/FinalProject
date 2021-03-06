#ifndef main_H_
#define main_H_

#include "gameplay.h"
#include <vector>
#include <map>
#include <string>
#include "..\MayaData.h"
#include "..\OurMaterial.h"



using namespace gameplay;

/**
 * Main game class.
 */
class main: public Game
{
public:

    /**
     * Constructor.
     */
    main();
	Vector4 main::getBLAH() const;
    /**
     * @see Game::keyEvent
     */
	void keyEvent(Keyboard::KeyEvent evt, int key);
	
    /**
     * @see Game::touchEvent
     */
    void touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex);

protected:

    /**
     * @see Game::initialize
     */
    void initialize();

    /**
     * @see Game::finalize
     */
    void finalize();

    /**
     * @see Game::update
     */
    void update(float elapsedTime);

    /**
     * @see Game::render
     */
    void render(float elapsedTime);

private:

    /**
     * Draws the scene each frame.
     */
    bool drawScene(Node* node);
    Scene* _scene;
    bool _wireframe;

	MayaData mayaData;

	std::map<std::string, VertexLayout*> meshVertecies;
	std::map<std::string, Mesh*> meshes;

	std::map<std::string, OurMaterial> ourMaterialMap;
	std::map<std::string, std::string> NodeIDToMaterial;

};

#endif
