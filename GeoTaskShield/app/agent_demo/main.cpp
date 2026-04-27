#include "agent/ExperimentAgent.h"

#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    std::string request = "对比三种隐私机制，50个用户，20个任务，使用匈牙利算法";
    if (argc > 1) {
        request.clear();
        for (int i = 1; i < argc; ++i) {
            if (!request.empty()) {
                request += ' ';
            }
            request += argv[i];
        }
    }

    const gts::ExperimentAgent agent;
    const gts::ExperimentAgentResult result = agent.run(request);
    std::cout << result.markdown;
    return 0;
}
