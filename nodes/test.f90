module test
 use iso_c_binding
 implicit none
!in
    real*8, allocatable, target :: inarray (:,:)
!in
    integer :: axis
!out
    real*8, allocatable, target :: myarray (:)

 contains
!entry
    subroutine Execute()
        myarray = inarray(axis,:)
        print*, "Fortran says hello!"
    end subroutine Execute
end module test