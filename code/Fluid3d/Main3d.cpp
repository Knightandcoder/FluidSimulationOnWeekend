#include <iostream>
#include "RenderWidget.h"
#include <thread>
#include "Global.h"
#include "ParticalSystem3d.h"

struct test {
    float a[200];
};

int main() {

    Fluid3d::ParticalSystem3D ps;
    ps.SetContainerSize(glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.3, 0.3, 0.3));
    ps.AddFluidBlock(glm::vec3(0.2, 0.0, 0.15), glm::vec3(0.1, 0.25, 0.15), glm::vec3(0.0, 0.5, 0.0), 0.01 * 0.9);
    ps.AddFluidBlock(glm::vec3(0.0, 0.1, 0.05), glm::vec3(0.1, 0.2, 0.2), glm::vec3(0.5, -0.5, -0.5), 0.01 * 0.9);
    ps.UpdateData();
    std::cout << "partical num = " << ps.mParticalInfos.size() << std::endl;
    
    Fluid3d::RenderWidget renderer;
    renderer.Init();
    renderer.UploadUniforms(ps);
    
    while (!renderer.ShouldClose()) {
        renderer.ProcessInput();    // ���������¼�
        ps.UpdateData();
        for (int i = 0; i < 8; i++) {
            renderer.UploadParticalInfo(ps);
            renderer.SolveParticals();
            renderer.DumpParticalInfo(ps);
        }
        renderer.Update();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        renderer.PollEvents();
    }
    
    return 0;
}