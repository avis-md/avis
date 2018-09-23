module test
 USE ISO_C_BINDING
 implicit none
    
!in
    real*8, allocatable, target :: darr (:,:)
    
!out
    real*8, allocatable, target :: myarray (:)
    
 contains
!entry
    subroutine Execute()
        print*, 'hello from fortran!', shape(darr)
    end subroutine Execute
    
end module test