; ModuleID = 'potatoscript'
source_filename = "potatoscript"

@0 = private unnamed_addr constant [15 x i8] c"Hello world!\\n\00", align 1
@1 = private unnamed_addr constant [13 x i8] c"Bye world!\\n\00", align 1

declare i32 @printf(i8* %a)

define i32 @main() {
entry:

  br i1 true, label %trueblock, label %falseblock

trueblock:                                        ; preds = %entry
  %calltmp2 = call i32 @printf(i8* getelementptr inbounds ([13 x i8], [13 x i8]* @1, i32 0, i32 0))
  ret i32 0

falseblock:                                       ; preds = %entry
  br label %ifcontinue

ifcontinue:                                       ; preds = %entry
  %calltmp = call i32 @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i32 0, i32 0))
  ret i32 0

}