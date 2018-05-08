#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

/**
  Name: Thomas Khuu

  Creates a function that computes the displacement.
  X_0 + V_0 * t - g*t^2 / 2
 */

static Function *createDisplacement(Module *M, LLVMContext &Context) {
    Function *disFunc = cast<Function>(M->getOrInsertFunction("add",
                                          Type::getDoubleTy(Context),
                                          Type::getDoubleTy(Context),
                                          Type::getDoubleTy(Context),
                                          Type::getDoubleTy(Context))
                                      );

    Function::arg_iterator args = &(*disFunc->arg_begin());
    Value *X_0 = args++;
    X_0->setName("x_0");
    Value *V_0 = args++;
    V_0->setName("v_0");
    Value *t = args++;
    t->setName("t");

    BasicBlock *entryBB = BasicBlock::Create(Context, "entry", disFunc);

    // Constants: gravity and 2
    Value *g = ConstantFP::get(Type::getDoubleTy(Context), 9.78033);
    //Value *two = ConstantFP::get(Type::getDoubleTy(Context), 2);    

    // Convert to double (alternative way)
    Value *two = ConstantInt::get(Type::getInt32Ty(Context), 2);    
    SIToFPInst *twoFP = new SIToFPInst(two, Type::getDoubleTy(Context), 
                                        "", entryBB);

    // V_0 * t
    Value *second = BinaryOperator::Create(Instruction::FMul, V_0, t, 
                                                          "", entryBB);

    // X_0 + V_0 * t
    Value *firstSecond = BinaryOperator::Create(Instruction::FAdd, X_0, second,
                                                               "", entryBB);

    // g * t^2 / 2
    Value *timeX2 = BinaryOperator::Create(Instruction::FMul, t, t,
                                                          "", entryBB);
    Value *third = BinaryOperator::Create(Instruction::FMul, g, timeX2,
                                                         "", entryBB);
    Value *thirdFinal = BinaryOperator::Create(Instruction::FDiv, third, twoFP,
                                                              "", entryBB);
    

    Value *Result = BinaryOperator::Create(Instruction::FSub, firstSecond,
                                           thirdFinal, "", entryBB);

    ReturnInst::Create(Context, Result, entryBB);

    return disFunc;
}

int main(int argc, char **argv) {
    double x_0 = argc >= 2 ? atof(argv[1]) : 10.0;
    double v_0 = argc >= 3 ? atof(argv[2]) : 0.0;
    double t   = argc >= 4 ? atof(argv[3]) : 1.0;

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    
    LLVMContext Context;

    std::unique_ptr<Module> mod(new Module("main", Context));
    Module *M = mod.get();
    Function *displacementFunc = createDisplacement(M, Context);


    std::string errStr;
    ExecutionEngine *EE = EngineBuilder(std::move(mod))
                                    .setErrorStr(&errStr)
                                    .create();
    if (!EE) {
        errs() << argv[0] << ": Failed to construct ExecutionEngine: " << errStr
               << "\n";
        return 1;
    }

    if (verifyModule(*M)) {
        errs() << argv[0] << ": Error constructing function!\n";
        return 1;
    }

    errs() << "starting displacement(" << x_0 << ", " << v_0 << ", " << t 
           <<  "):\n";
    
    uint64_t func = EE->getFunctionAddress(displacementFunc->getName().str());
    double (*func_ptr)(double, double, double) = 
                      (double (*)(double, double, double)) func;

    outs() << "Output: " << func_ptr(x_0, v_0, t) << "\n";
    return 0;
}
