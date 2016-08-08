
#include <iDynTree/Core/Axis.h>
#include <iDynTree/KinDynComputations.h>
#include <iDynTree/Model/JointState.h>
#include <iDynTree/Model/Indeces.h>
#include <iDynTree/Model/Model.h>
#include <iDynTree/Model/RevoluteJoint.h>

#include <iDynTree/ModelIO/ModelLoader.h>

#include <yarp/os/Property.h>
#include <cmath>
#include <cstdlib>

inline bool checkDoubleAreEqual(const double & val1,
                                const double & val2,
                                const double tol)
{
    if( std::fabs(val1-val2) > tol )
    {
        return false;
    }

    return true;
}


template<typename VectorType1, typename VectorType2>
bool checkVectorAreEqual(const VectorType1 & dir1,
                         const VectorType2 & dir2,
                         const double tol)
{
    if( dir1.size() != dir2.size() )
    {
        return false;
    }

    for(int i=0; i < dir1.size(); i++)
    {
        if( std::fabs(dir1(i)-dir2(i)) > tol )
        {
            return false;
        }
    }
    return true;
}

template<typename MatrixType1, typename MatrixType2>
bool checkMatrixAreEqual(const MatrixType1 & mat1,
                         const MatrixType2 & mat2,
                         const double tol)
{
    if( mat1.rows() != mat2.rows() ||
        mat1.cols() != mat2.cols() )
    {
        return false;
    }

    for(int i=0; i < mat1.rows(); i++)
    {
        for(int j=0; j < mat1.cols(); j++ )
        {
            if( std::fabs(mat1(i,j)-mat2(i,j)) > tol )
            {
                return false;
            }
        }
    }
    return true;
}

bool checkTransformAreEqual(const iDynTree::Transform & t1,
                            const iDynTree::Transform & t2,
                            const double tol)
{
    return checkMatrixAreEqual(t1.getRotation(),t2.getRotation(),tol) &&
           checkVectorAreEqual(t1.getPosition(),t2.getPosition(),tol);
}


bool getAxisInRootLink(iDynTree::KinDynComputations & comp,
                                 const std::string jointName,
                                 iDynTree::Axis & axisInRootLink)
{
    iDynTree::LinkIndex rootLinkIdx = comp.getFrameIndex("root_link");

    if( rootLinkIdx == iDynTree::FRAME_INVALID_INDEX )
    {
        std::cerr << "icub-model-test error: impossible to find root_link in model" << std::endl;
        return false;
    }

    iDynTree::JointIndex jntIdx = comp.getRobotModel().getJointIndex(jointName);

    if( jntIdx == iDynTree::JOINT_INVALID_INDEX )
    {
        std::cerr << "icub-model-test error: impossible to find " << jointName << " in model" << std::endl;
        return false;
    }

    iDynTree::LinkIndex childLinkIdx = comp.getRobotModel().getJoint(jntIdx)->getSecondAttachedLink();

    // Check that the joint are actually revolute as all the joints in iCub
    const iDynTree::RevoluteJoint * revJoint = dynamic_cast<const iDynTree::RevoluteJoint *>(comp.getRobotModel().getJoint(jntIdx));

    if( !revJoint )
    {
        std::cerr << "icub-model-test error: " << jointName << " is not revolute " << std::endl;
        return false;
    }

    axisInRootLink = comp.getRelativeTransform(rootLinkIdx,childLinkIdx)*(revJoint->getAxis(childLinkIdx));

    return true;
}

bool checkBaseLink(iDynTree::KinDynComputations & comp)
{
    iDynTree::LinkIndex rootLinkIdx = comp.getFrameIndex("root_link");

    if( rootLinkIdx == iDynTree::FRAME_INVALID_INDEX )
    {
        std::cerr << "icub-model-test error: impossible to find root_link in model" << std::endl;
        return false;
    }

    iDynTree::LinkIndex base_linkIdx = comp.getFrameIndex("base_link");

    if( rootLinkIdx == iDynTree::FRAME_INVALID_INDEX )
    {
        std::cerr << "icub-model-test error: impossible to find base_link in model" << std::endl;
        return false;
    }

    if( comp.getRobotModel().getFrameLink(base_linkIdx) != rootLinkIdx )
    {
        std::cerr << "icub-model-test error: base_link is not attached to root_link" << std::endl;
        return false;
    }

    if( !checkTransformAreEqual(comp.getRobotModel().getFrameTransform(base_linkIdx),iDynTree::Transform::Identity(),1e-6) )
    {
        std::cerr << "icub-model-test error: base_link <---> root_link transform is not an identity" << std::endl;
        return false;
    }

    std::cerr << "icub-model-test error: base_link test performed correctly " << std::endl;

    return true;
}

bool checkSolesAreParallel(iDynTree::KinDynComputations & comp)
{
    iDynTree::LinkIndex rootLinkIdx = comp.getFrameIndex("root_link");

    if( rootLinkIdx == iDynTree::FRAME_INVALID_INDEX )
    {
        std::cerr << "icub-model-test error: impossible to find root_link in model" << std::endl;
        return false;
    }

    iDynTree::LinkIndex l_sole = comp.getFrameIndex("l_sole");

    if( rootLinkIdx == iDynTree::FRAME_INVALID_INDEX )
    {
        std::cerr << "icub-model-test error: impossible to find frame l_sole in model" << std::endl;
        return false;
    }

    iDynTree::LinkIndex r_sole = comp.getFrameIndex("r_sole");

    if( rootLinkIdx == iDynTree::FRAME_INVALID_INDEX )
    {
        std::cerr << "icub-model-test error: impossible to find frame r_sole in model" << std::endl;
        return false;
    }

    iDynTree::Transform root_H_l_sole = comp.getRelativeTransform(rootLinkIdx,l_sole);
    iDynTree::Transform root_H_r_sole = comp.getRelativeTransform(rootLinkIdx,r_sole);

    double l_sole_height = root_H_l_sole.getPosition().getVal(2);
    double r_sole_height = root_H_r_sole.getPosition().getVal(2);

    if( !checkDoubleAreEqual(l_sole_height,r_sole_height,1e-8) )
    {
        std::cerr << "icub-model-test error: l_sole_height is " << l_sole_height << ", while r_sole_height is " << r_sole_height << " (diff : " << std::fabs(l_sole_height-r_sole_height) <<  " )"  << std::endl;
        return false;
    }

    std::cerr << "icub-model-test error: sole are parallel test performed correctly " << std::endl;

    return true;
}



bool checkAxisDirections(iDynTree::KinDynComputations & comp)
{
    // Check axis for the first thing
    // see https://github.com/robotology/codyco-superbuild/issues/55#issuecomment-227872325 and
    // http://wiki.icub.org/wiki/ICub_Model_naming_conventions    // Check axis for the first thing
    // see https://github.com/robotology/codyco-superbuild/issues/55#issuecomment-227872325 and
    // http://wiki.icub.org/wiki/ICub_Model_naming_conventions

    std::vector<std::string> axisNames;
    std::vector<iDynTree::Direction> expectedDirectionInRootLink;

    axisNames.push_back("torso_pitch");
    expectedDirectionInRootLink.push_back(iDynTree::Direction(0.0,-1.0,0.0));
    axisNames.push_back("torso_roll");
    expectedDirectionInRootLink.push_back(iDynTree::Direction(1.0,0.0,0.0));
    axisNames.push_back("torso_yaw");
    expectedDirectionInRootLink.push_back(iDynTree::Direction(0.0,0.0,-1.0));

    for(int i=0; i < axisNames.size(); i++)
    {
        std::string axisToCheck = axisNames[i];
        iDynTree::Axis axisInRootLink;
        iDynTree::Direction expectedDirection = expectedDirectionInRootLink[i];
        bool getAxisOk = getAxisInRootLink(comp,axisToCheck,axisInRootLink);

        if( !getAxisOk )
        {
            return false;
        }

        if( !checkVectorAreEqual(axisInRootLink.getDirection(),expectedDirection,1e-5) )
        {
            std::cerr << "icub-model-test error:" << axisToCheck << " got direction of " << axisInRootLink.getDirection().toString()
                  << " instead of expected " << expectedDirection.toString() << std::endl;
            return false;
        }
    }

    return true;
}

/**
 * All the iCub have a even and not null number of F/T sensors.
 */
bool checkFTSensorsAreEvenAndNotNull(iDynTree::ModelLoader & mdlLoader)
{
    int nrOfFTSensors = mdlLoader.sensors().getNrOfSensors(iDynTree::SIX_AXIS_FORCE_TORQUE);

    if( nrOfFTSensors == 0 )
    {
        std::cerr << "icub-model-test error: no F/T sensor found in the model" << std::endl;
        return false;
    }

    if( nrOfFTSensors % 2 == 1 )
    {
        std::cerr << "icub-model-test error: odd number of F/T sensor found in the model" << std::endl;
        return false;
    }


    return true;
}

int main(int argc, char ** argv)
{
    yarp::os::Property prop;

    // Parse command line options
    prop.fromCommand(argc,argv);

    // Skip upper body tests (to support testing iCubHeidelberg01 model)
    bool skipUpperBody = prop.check("skipUpperBody");

    // Get model name
    if( ! prop.find("model").isString() )
    {
        std::cerr << "icub-model-test error: model option not found" << std::endl;
        return EXIT_FAILURE;
    }

    std::string modelPath = prop.find("model").asString().c_str();

    // Open the model
    iDynTree::KinDynComputations comp;

    bool modelLoaded = comp.loadRobotModelFromFile(modelPath);

    if( !modelLoaded )
    {
        std::cerr << "icub-model-test error: impossible to load model from " << modelLoaded << std::endl;
        return EXIT_FAILURE;
    }

    iDynTree::Vector3 grav;
    grav.zero();
    iDynTree::JointPosDoubleArray qj(comp.getRobotModel());
    iDynTree::JointDOFsDoubleArray dqj(comp.getRobotModel());
    qj.zero();
    dqj.zero();

    comp.setRobotState(qj,dqj,grav);

    // Check axis
    if( !checkAxisDirections(comp) )
    {
        return EXIT_FAILURE;
    }

    // Check if base_link exist, and check that is a frame attached to root_link and if its
    // transform is the idyn
    if( !checkBaseLink(comp) )
    {
        return EXIT_FAILURE;
    }


    // Check if l_sole/r_sole have the same distance from the root_link
    if( !checkSolesAreParallel(comp) )
    {
        return EXIT_FAILURE;
    }

    // Now some test that test the sensors
    iDynTree::ModelLoader mdlLoader;
    mdlLoader.loadModelFromFile(modelPath);


    if( !checkFTSensorsAreEvenAndNotNull(mdlLoader) )
    {
        return EXIT_FAILURE;
    }

    std::cerr << "Check for model " << modelPath << " concluded correctly!" << std::endl;

    return EXIT_SUCCESS;
}
