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
  
  First usage adding 2 numbers from function arguments.
 */

static Function *createAddFunc(Module *M, LLVMContext &Context) {

    // Create and insert the add function into the module
    // The add function takes 2 ints parameters to add up
    Function *addFunc = cast<Function>(M->getOrInsertFunction("add",
                                          Type::getInt32Ty(Context),
                                          Type::getInt32Ty(Context),
                                          Type::getInt32Ty(Context))
                                      );

    Function::arg_iterator args = &(*addFunc->arg_begin());
    Value *x = args++;
    x->setName("x");
    Value *y = args++;
    y->setName("y");

    BasicBlock *entryBB = BasicBlock::Create(Context, "entry", addFunc);

    Value *Sum = BinaryOperator::CreateAdd(x, y, "result", entryBB);

    ReturnInst::Create(Context, Sum, entryBB);

    return addFunc;
}

int main(int argc, char **argv) {
    int i = argc >= 2 ? atol(argv[1]) : 200;
    int j = argc >= 3 ? atol(argv[2]) : -300;

    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    
    LLVMContext Context;

    std::unique_ptr<Module> Owner(new Module("test", Context));
    Module *M = Owner.get();
    Function *addFunc = createAddFunc(M, Context);


    std::string errStr;
    ExecutionEngine *EE = EngineBuilder(std::move(Owner))
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

    errs() << "starting add(" << i << ", " << j << "):\n";
    
    uint64_t func = EE->getFunctionAddress(addFunc->getName().str());
    int (*func_ptr)(int, int) = (int (*)(int, int)) func;

    outs() << "Output: " << func_ptr(i, j) << "\n";
    return 0;
}
