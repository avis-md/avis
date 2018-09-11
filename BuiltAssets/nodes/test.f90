module test
 use iso_c_binding
 implicit none
!in
    integer :: inint = 0
!out
    real*8, allocatable, target :: myarray (:)

 contains
!entry
    subroutine Execute()
        integer :: i
        if (allocated(myarray)) &
            deallocate(myarray)
        allocate(myarray(0:99))
        do i = 0, 99
        myarray(i) = sin(i * (inint + 1) * 2 * 3.14159d-2)
        end do
        print*, "Fortran says hello!"
    end subroutine Execute
end module test