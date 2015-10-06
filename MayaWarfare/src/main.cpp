#include "main.h"

// Declare our game instance
main game;

main::main()
    : _scene(NULL), _wireframe(false)
{
}

void main::initialize()
{
	shared.initialize(200 * 1024 * 1024, (LPCWSTR)"MayaToGameEngine", false);
	data = nullptr;
	dataSize = 0;
	// Load game scene from file
    _scene = Scene::load("res/demo.scene");
    // Get the box model and initialize its material parameter values and bindings
    Node* boxNode = _scene->findNode("box");
    Model* boxModel = dynamic_cast<Model*>(boxNode->getDrawable());
    Material* boxMaterial = boxModel->getMaterial();


    // Set the aspect ratio for the scene's camera to match the current resolution

    _scene->getActiveCamera()->setAspectRatio(getAspectRatio());
}

void main::finalize()
{
    SAFE_RELEASE(_scene);
}

void main::update(float elapsedTime)
{
	MessageType type;
	INT64 length;

	type = shared.Read(data, dataSize, length);
	if (type)
	{
		if (type == MessageType::mNewMesh)
		{
			Node* boxNode = _scene->findNode("box");
			Model* boxModel = dynamic_cast<Model*>(boxNode->getDrawable());
			Material* boxMaterial = boxModel->getMaterial();

			MeshHeader* header = (MeshHeader*)data;
			//char* name = &data[sizeof(MeshHeader)];
			char* name = new char[header->nameLength];
			memcpy(name, &data[sizeof(MeshHeader)], header->nameLength);

			Node* triNode = Node::create(name);
			VertexFormat::Element element(VertexFormat::POSITION, 3);
			const VertexFormat vertFormat(&element, 1);

			nodeNames.push_back(name);

			//VertexLayout* verteciesData = new VertexLayout[header->vertexCount];
			//memcpy(verteciesData, &data[sizeof(MeshHeader)+header->nameLength], (header->vertexCount * sizeof(VertexLayout)));
			////VertexLayout* verteciesData = (VertexLayout*)&data[sizeof(MeshHeader)+header->nameLength];

			////UINT* index = (UINT*)&data[sizeof(MeshHeader)+header->nameLength + (header->vertexCount * sizeof(VertexLayout))];
			//UINT* index = new UINT[header->indexCount];
			//memcpy(index, &data[sizeof(MeshHeader)+header->nameLength + (header->vertexCount * sizeof(VertexLayout))], header->indexCount * sizeof(UINT));
			//Mesh* triMesh = Mesh::createMesh(vertFormat, header->vertexCount, false);

			//triMesh->setVertexData((float*)verteciesData, 0, header->vertexCount);

			//MeshPart* test = triMesh->addPart(Mesh::PrimitiveType::TRIANGLES, Mesh::IndexFormat::INDEX32, header->indexCount, false);

			//test->setIndexData(index, 0, header->indexCount);



			//-------------TEST!----------------
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
			//-------------END TEST!------------
			
			
			Model* triModel = Model::create(triMesh);
			triModel->setMaterial(boxMaterial);
			triNode->setDrawable(triModel);
			triNode->setEnabled(true);
			_scene->addNode(triNode);
		}
	}
	// Rotate model
	//_scene->findNode("box")->translateX(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
	for (std::vector<char*>::iterator it = nodeNames.begin(); it != nodeNames.end(); ++it)
	{
		_scene->findNode(*it)->rotateX(MATH_DEG_TO_RAD((float)elapsedTime / 1000.0f * 180.0f));
	}
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
