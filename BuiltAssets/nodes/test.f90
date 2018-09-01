module test
 USE ISO_C_BINDING
 implicit none
    
!out
    integer :: myint = 5
!out
    real*8 :: mydouble = 0.5d+0
!out
    real*8, allocatable, target :: myarray (:)
    
 contains
!entry
    subroutine Execute()
        print*, 'hello from fortran!'
    end subroutine Execute
    
end module test