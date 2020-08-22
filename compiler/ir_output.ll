; ModuleID = 'potatoscript'
source_filename = "potatoscript"

@0 = private unnamed_addr constant [15 x i8] c"Hello world!\\n\00", align 1

declare float @printf(i8*)

define i32 @main() {
entry:

  %calltmp = call float @printf(i8* getelementptr inbounds ([15 x i8], [15 x i8]* @0, i32 0, i32 0))
  ret i32 0
}