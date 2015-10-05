#include "main.h"

// Declare our game instance
main game;

main::main()
    : _scene(NULL), _wireframe(false)
{
}

void main::initialize()
{
    // Load game scene from file
    _scene = Scene::load("res/demo.scene");

    // Get the box model and initialize its material parameter values and bindings
    Node* boxNode = _scene->findNode("box");
    Model* boxModel = dynamic_cast<Model*>(boxNode->getDrawable());
    Material* boxMaterial = boxModel->getMaterial();

	Node* triNode = Node::create("tri");
	triNode->setTranslationX(0);
	triNode->setTranslationY(0);
	triNode->setTranslationZ(0);
	Matrix rot; 
	Matrix::createRotationX(90, &rot);
	triNode->setRotation(rot);
	VertexFormat::Element element(VertexFormat::POSITION, 3);
	const VertexFormat vertFormat(&element, 1);

	Mesh* triMesh = Mesh::createMesh(vertFormat, 4, false);
	float vertData[]{
		1.0f, 0.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f };
	int indexData[]{ 0, 1, 2, 1, 2, 3 };
	triMesh->setVertexData(vertData);
	MeshPart* test = triMesh->addPart(Mesh::TRIANGLES, Mesh::IndexFormat::INDEX32, 6, false);
	test->setIndexData(indexData, 0, 6);
	//Vector3 p1(1, 0, 0);
	//Vector3 p2(1, 1, 0);
	//Vector3 p3(0, 0, 0);
	//Vector3 p4(0, 1, 0);
	//triMesh = Mesh::createQuad(p1, p2, p3, p4);

	
	Model* triModel = Model::create(triMesh);
	triModel->setMaterial(boxMaterial);
	triNode->setDrawable(triModel);
	triNode->setEnabled(true);

    // Set the aspect ratio for the scene's camera to match the current resolution
	_scene->addNode(triNode);
    _scene->getActiveCamera()->setAspectRatio(getAspectRatio());
}

void main::finalize()
{
    SAFE_RELEASE(_scene);
}

void main::update(float elapsedTime)
{
    // Rotate model
	_scene->findNode("box")->translateX(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
	_scene->findNode("tri")->rotateY(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
}

void main::render(float elapsedTime)
{
    // Clear the color and depth buffers
    clear(CLEAR_COLOR_DEPTH, Vector4::zero(), 1.0f, 0);

    // Visit all the nodes in the scene for drawing
    _scene->visit(this, &main::drawScene);
}

bool main::drawScene(Node* node)
{
    // If the node visited contains a drawable object, draw it
    Drawable* drawable = node->getDrawable(); 
    if (drawable)
        drawable->draw(_wireframe);

    return true;
}

void main::keyEvent(Keyboard::KeyEvent evt, int key)
{
    if (evt == Keyboard::KEY_PRESS)
    {
        switch (key)
        {
        case Keyboard::KEY_ESCAPE:
            exit();
            break;
        }
    }
}

void main::touchEvent(Touch::TouchEvent evt, int x, int y, unsigned int contactIndex)
{
    switch (evt)
    {
    case Touch::TOUCH_PRESS:
        _wireframe = !_wireframe;
        break;
    case Touch::TOUCH_RELEASE:
        break;
    case Touch::TOUCH_MOVE:
        break;
    };
}
