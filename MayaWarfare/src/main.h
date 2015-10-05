#ifndef main_H_
#define main_H_

#include "gameplay.h"
#include <vector>
#include "..\..\Maya\MayaToGameEngine\Memory.h"
#include "..\..\Maya\MayaToGameEngine\sharedMemory.h"

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
	std::vector<char*> nodeNames;
	Memory mem;
	SharedMemory shared;
};

#endif