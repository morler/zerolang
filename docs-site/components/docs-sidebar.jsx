"use client";

import Link from "next/link";
import { useState } from "react";
import { ChevronDownIcon, CloseIcon, HamburgerIcon } from "@/components/icons";

export function DocsSidebarShell({ groups, activeSlug }) {
  const initialCollapsed = new Set(
    groups
      .filter((g) => g.section !== "Learn" && !g.items.some((i) => i.slug === activeSlug))
      .map((g) => g.section),
  );
  const [collapsed, setCollapsed] = useState(initialCollapsed);
  const [menuOpen, setMenuOpen] = useState(false);

  function toggle(section) {
    setCollapsed((prev) => {
      const next = new Set(prev);
      if (next.has(section)) next.delete(section);
      else next.add(section);
      return next;
    });
  }

  function closeMenu() {
    setMenuOpen(false);
  }

  return (
    <>
      {/* Mobile topbar */}
      <div className="sticky top-0 z-40 flex h-14 items-center justify-end border-b border-border bg-bg px-4 md:hidden">
        <button
          type="button"
          aria-label="Toggle menu"
          onClick={() => setMenuOpen(true)}
          className="flex h-8 w-8 cursor-pointer items-center justify-center rounded text-fg hover:bg-surface-muted"
        >
          <HamburgerIcon />
        </button>
      </div>

      {/* Backdrop */}
      {menuOpen ? (
        <div
          className="fixed inset-0 z-40 bg-black/40 md:hidden"
          onClick={closeMenu}
          aria-hidden="true"
        />
      ) : null}

      {/* Sidebar */}
      <aside
        aria-label="Documentation"
        className={`fixed left-0 top-0 z-50 h-screen w-72 max-w-[85vw] overflow-y-auto bg-bg transition-transform md:sticky md:top-0 md:w-60 md:max-w-none md:translate-x-0 ${
          menuOpen ? "translate-x-0 shadow-[4px_0_24px_rgb(0_0_0/0.15)]" : "-translate-x-full"
        }`}
      >
        <div className="sticky top-0 z-10 flex items-center justify-end border-b border-border bg-bg px-6 py-4 md:hidden">
          <button
            type="button"
            aria-label="Close menu"
            onClick={closeMenu}
            className="flex h-8 w-8 cursor-pointer items-center justify-center rounded text-fg hover:bg-surface-muted"
          >
            <CloseIcon />
          </button>
        </div>

        <nav className="px-3 py-4">
          {groups.map((group) => {
            const hasActive = group.items.some((item) => item.slug === activeSlug);
            const isCollapsed = collapsed.has(group.section) && !hasActive;
            return (
              <div key={group.section} className="mb-4">
                <button
                  type="button"
                  aria-expanded={!isCollapsed}
                  onClick={() => toggle(group.section)}
                  className="flex w-full cursor-pointer items-center justify-between rounded bg-transparent px-3 py-2 text-[0.6875rem] font-semibold uppercase tracking-[0.06em] text-muted transition hover:text-fg"
                >
                  <span>{group.section}</span>
                  <ChevronDownIcon className={`shrink-0 transition-transform ${isCollapsed ? "-rotate-90" : ""}`} />
                </button>
                <div
                  className={`grid transition-[grid-template-rows] duration-200 ${
                    isCollapsed ? "grid-rows-[0fr]" : "grid-rows-[1fr]"
                  }`}
                >
                  <div className="overflow-hidden">
                    {group.items.map((item) => {
                      const active = item.slug === activeSlug;
                      return (
                        <Link
                          key={item.slug}
                          href={item.path}
                          aria-current={active ? "page" : undefined}
                          onClick={closeMenu}
                          className={`block rounded px-3 py-[0.1875rem] text-[0.8125rem] leading-[1.8] no-underline transition hover:text-fg ${
                            active ? "font-medium text-fg" : "text-muted"
                          }`}
                        >
                          {item.title}
                        </Link>
                      );
                    })}
                  </div>
                </div>
              </div>
            );
          })}
        </nav>
      </aside>
    </>
  );
}
