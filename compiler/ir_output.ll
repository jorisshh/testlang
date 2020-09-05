; ModuleID = 'potatoscript'
source_filename = "potatoscript"

%Vec3 = type { float, float, float }

@0 = private unnamed_addr constant [15 x i8] c"Hello world!\\n\00", align 1
@1 = private unnamed_addr constant [9 x i8] c"oh no...\00", align 1
@2 = private unnamed_addr constant [14 x i8] c"Oh lawd its 1\00", align 1
@3 = private unnamed_addr constant [13 x i8] c"Bye world!\\n\00", align 1
@4 = private unnamed_addr constant [8 x i8] c"works??\00", align 1

declare void @printf(i8* %a)

declare i32 @sqrtf(float %a)

declare i32 @sinf(float %a)

declare i32 @cosf(float %a)

define i32 @testFunc(i32 %a) {
entry:
  %gttmp = icmp sgt i32 %a, 1
  br i1 %gttmp, label %trueblock, label %falseblock

trueblock:                                        ; preds = %entry
  ret i32 1

falseblock:                                       ; preds = %entry
  br label %ifcontinue

ifcontinue:                                       ; preds = %falseblock
  ret i32 0
}

define void @logTest(i8* %a) {
entry:
  call void @printf(i8* %a)
  ret void
}

define i32 @takesVec3(%Vec3 %v) {
entry:
  ret void
}

define i32 @main() {
entry:
  br i1 true, label %trueblock, label %falseblock

trueblock:                                        ; preds = %entry
  call void @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i32 0, i32 0))
  br label %ifcontinue

falseblock:                                       ; preds = %entry
  br label %ifcontinue

ifcontinue:                                       ; preds = %falseblock, %trueblock
  br i1 false, label %trueblock1, label %falseblock2

trueblock1:                                       ; preds = %ifcontinue
  call void @printf(i8* getelementptr inbounds ([9 x i8], [9 x i8]* @1, i32 0, i32 0))
  br label %ifcontinue3

falseblock2:                                      ; preds = %ifcontinue
  br label %ifcontinue3

ifcontinue3:                                      ; preds = %falseblock2, %trueblock1
  %calltmp = call i32 @testFunc(i32 3)
  %eqtmp = icmp eq i32 %calltmp, 1
  br i1 %eqtmp, label %trueblock4, label %falseblock5

trueblock4:                                       ; preds = %ifcontinue3
  call void @printf(i8* getelementptr inbounds ([14 x i8], [14 x i8]* @2, i32 0, i32 0))
  br label %ifcontinue6

falseblock5:                                      ; preds = %ifcontinue3
  br label %ifcontinue6

ifcontinue6:                                      ; preds = %falseblock5, %trueblock4
  call void @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @3, i32 0, i32 0))
  call void @logTest(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @4, i32 0, i32 0))
  ret i32 0
}
